#include "client.h"

#include "../../event/event_loop.h"
#include "../../event/timer.h"
#include "../../util/str.h"

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

namespace tcp {

Result<std::shared_ptr<ClientConnection>> Client::get_conn_hdl() {
    addrinfo* addrs;

    addrinfo hints = {
        .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM, .ai_protocol = 0};

    int err = getaddrinfo(m_host.c_str(), std::to_string(m_port).c_str(),
                          &hints, &addrs);

    if (err != 0) {
        return Result<std::shared_ptr<ClientConnection>>::Err(
            Error("failed to resolve host"));
    }

    std::shared_ptr<ClientConnection> hdl = ClientConnection::create(
        m_logger_context, addrs, addrs, m_connect_cb, m_fail_cb);

    return Result<std::shared_ptr<ClientConnection>>::Ok(hdl);
}

ClientConnection::ClientConnection(
    Private, std::shared_ptr<LoggerContext> logger_context, addrinfo* addrs,
    addrinfo* curr_addr, Client::ConnectCallback connect_cb,
    Client::FailCallback fail_cb)
    : IOHandle(EPOLLET | EPOLLOUT | EPOLLIN | EPOLLRDHUP | EPOLLHUP),
      m_logger_context(logger_context),
      m_logger(logger_context->use("TCPConn")), m_connected(false),
      m_addrs(addrs), m_curr_addr(curr_addr),
      m_addr_str(util::str::addr_to_str(curr_addr)), m_connect_cb(connect_cb),
      m_fail_cb(fail_cb) {

    if (m_curr_addr == nullptr) {
        m_fd_result = Result<int>::Err(Error("bad address"));
        return;
    }

    int fd =
        socket(m_curr_addr->ai_family, m_curr_addr->ai_socktype | SOCK_NONBLOCK,
               m_curr_addr->ai_protocol);

    if (fd < 0) {
        m_fd_result = Result<int>::Err(Error("failed to create socket fd"));
        return;
    }

    if (connect(fd, m_curr_addr->ai_addr, m_curr_addr->ai_addrlen) != 0 &&
        errno != EINPROGRESS) {
        ::close(fd);
        m_fd_result = Result<int>::Err(Error("connect failed"));
        return;
    }

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(fd);
}

ClientConnection::~ClientConnection() {
    if (m_addrs != nullptr && m_curr_addr != nullptr &&
        m_curr_addr->ai_next != nullptr) {
        freeaddrinfo(m_addrs);
    }
}

Result<int> ClientConnection::send(const std::vector<uint8_t>& data) {
    if (!m_fd_result.is_ok()) {
        return Result<int>::Err(
            Error("fd error: " + m_fd_result.unwrap_err().get_msg()));
    }

    int fd = m_fd_result.unwrap();

    int bytes_sent = ::send(fd, &data[0], data.size(), 0);
    if (bytes_sent < 0) {
        m_logger.debug("send(): error: " + std::string(strerror(errno)));
        return Result<int>::Err(Error("send failed"));
    }

    m_logger.verbose("send(): sent " + std::to_string(bytes_sent) + " bytes");
    return Result<int>::Ok(bytes_sent);
}

bool ClientConnection::on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                               const std::weak_ptr<Timer>& timer_hdl, int fd,
                               uint32_t events) {
    if (!m_connected && (events & EPOLLOUT)) {
        Result<bool> res = connect_next(event_loop_ptr, timer_hdl, fd);
        return res.is_ok() ? res.unwrap() : false;
    }

    if (m_connected && (events & EPOLLOUT)) {
        Result<std::vector<uint8_t>> res = recv(fd);
        if (!res.is_ok()) {
            m_logger.debug("[" + m_addr_str +
                           "]: recv failed: " + res.unwrap_err().get_msg());

            if (m_disconnect_cb) {
                m_disconnect_cb(get_ptr());
            }

            return false;
        }

        if (m_message_cb) {
            m_message_cb(get_ptr(), res.unwrap());
        }

        return true;
    }

    if (m_connected && (events & EPOLLIN)) {
        return true;
    }

    if (m_connected && (events & EPOLLRDHUP) ||
        m_connected && (events & EPOLLHUP)) {
        m_logger.verbose("[" + m_addr_str +
                         "]: connection closed by remote peer");

        if (m_disconnect_cb) {
            m_disconnect_cb(get_ptr());
        }

        return false;
    }

    if (m_connected && (events & EPOLLERR)) {
        m_logger.verbose("[" + m_addr_str +
                         "]: error: " + std::string(strerror(errno)));
        return false;
    }

    m_logger.verbose("[" + m_addr_str + "]: unhandled event");

    return false;
}

Result<bool>
ClientConnection::connect_next(const std::shared_ptr<EventLoop>& event_loop_ptr,
                               const std::weak_ptr<Timer>& timer_hdl, int fd) {
    int err;
    socklen_t err_len = sizeof(err);

    std::shared_ptr<Timer> timer = timer_hdl.lock();

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &err_len) < 0) {
        disable_timeout(timer);

        if (m_fail_cb) {
            m_fail_cb();
        }

        return Result<bool>::Err(Error("failed to read socket error state"));
    }

    if (err == 0) {
        m_logger.verbose("[" + m_addr_str + "]: connection established");

        if (m_connect_cb) {
            m_connect_cb(get_ptr());
        }

        m_connected = true;
        disable_timeout(timer);
        return Result<bool>::Ok(true);
    }

    if (m_curr_addr->ai_next == nullptr) {
        m_logger.verbose("[" + m_addr_str + "]: connection failed");

        if (m_fail_cb) {
            m_fail_cb();
        }

        disable_timeout(timer);
        return Result<bool>::Ok(false);
    }

    m_logger.verbose("[" + m_addr_str + "]: connection failed, trying next...");

    std::shared_ptr<ClientConnection> next_conn =
        ClientConnection::create(m_logger_context, m_addrs,
                                 m_curr_addr->ai_next, m_connect_cb, m_fail_cb);

    event_loop_ptr->add(next_conn, timer ? timer->get_timeout_ms() : -1);

    return Result<bool>::Ok(false);
}

void ClientConnection::disable_timeout(const std::shared_ptr<Timer>& timer) {
    if (timer) {
        m_logger.verbose("[" + m_addr_str + "]: disabled timeout");
        timer->disable();
    }
}

Result<std::vector<uint8_t>> ClientConnection::recv(int fd) {
    static const int CHUNK_SIZE = 4096;

    std::vector<uint8_t> buf;
    uint8_t chunk[CHUNK_SIZE];

    while (true) {
        int bytes_received = ::recv(fd, chunk, CHUNK_SIZE, 0);
        if (bytes_received > 0) {
            buf.insert(buf.end(), chunk, chunk + bytes_received);
        }

        else if (bytes_received == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            return Result<std::vector<uint8_t>>::Err(
                Error(std::string(strerror(errno))));
        }

        else {
            return Result<std::vector<uint8_t>>::Err(
                Error("connection closed"));
        }
    }

    m_logger.verbose("recv(): received " + std::to_string(buf.size()) +
                     " bytes");
    return Result<std::vector<uint8_t>>::Ok(buf);
}

} // namespace tcp