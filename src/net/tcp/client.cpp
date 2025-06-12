#include "client.h"

#include "../../event/event_loop.h"
#include "../../event/timer.h"
#include "../../util/str.h"
#include "../../util/unix.h"

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>

namespace tcp {

Result<std::shared_ptr<Client::Connection>> Client::get_conn_hdl() const {
    addrinfo* addrs;

    addrinfo hints = {
        .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM, .ai_protocol = 0};

    int err = getaddrinfo(m_host.c_str(), std::to_string(m_port).c_str(),
                          &hints, &addrs);

    if (err != 0) {
        return Result<std::shared_ptr<Connection>>::Err(
            Error(__func__, "failed to resolve host"));
    }

    std::shared_ptr<Connection> hdl = Connection::create(
        m_logger_context, addrs, addrs, m_connect_cb, m_fail_cb);

    return Result<std::shared_ptr<Connection>>::Ok(hdl);
}

Client::Connection::Connection(Client::Connection::Private,
                               std::shared_ptr<LoggerContext> logger_context,
                               addrinfo* addrs, addrinfo* curr_addr,
                               ConnectCallback connect_cb, FailCallback fail_cb)
    : tcp::Connection(EPOLLET | EPOLLIN | EPOLLOUT, logger_context,
                      "CTCPConnection"),
      m_connected(false), m_addrs(addrs), m_curr_addr(curr_addr),
      m_connect_cb(connect_cb), m_fail_cb(fail_cb) {

    if (m_curr_addr == nullptr) {
        m_fd_result = Result<int>::Err(Error(__func__, "bad address"));
        return;
    }

    int fd = socket(m_curr_addr->ai_family, m_curr_addr->ai_socktype,
                    m_curr_addr->ai_protocol);

    if (fd < 0) {
        m_fd_result = Result<int>::Err(Error::from_errno(__func__, "socket"));
        return;
    }

    Result<> nb_res = util::nix::set_fd_nonblock(fd);
    if (!nb_res.is_ok()) {
        ::close(fd);
        m_fd_result = Result<int>::Err(Error(__func__, nb_res));
        return;
    }

    if (connect(fd, m_curr_addr->ai_addr, m_curr_addr->ai_addrlen) != 0 &&
        errno != EINPROGRESS) {
        ::close(fd);
        m_fd_result = Result<int>::Err(Error::from_errno(__func__, "connect"));
        return;
    }

    m_addr_str = "???";

    Result<std::string> addr_res = util::str::addr_to_str(curr_addr);
    if (!addr_res.is_ok()) {
        m_logger.warn("Failed to convert address to string: " +
                      addr_res.unwrap_err());
    } else {
        m_addr_str = addr_res.unwrap();
    }

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(fd);
}

Client::Connection::~Connection() {
    if (m_addrs != nullptr && m_curr_addr != nullptr &&
        m_curr_addr->ai_next != nullptr) {
        freeaddrinfo(m_addrs);
    }
}

bool Client::Connection::is_connected(
    const std::shared_ptr<EventLoop>& event_loop_ptr,
    const std::weak_ptr<Timer>& timer_hdl, int fd, uint32_t events) {
    if (m_connected) {
        return true;
    }

    if (events & EPOLLOUT) {
        Result<bool> res = connect_next(event_loop_ptr, timer_hdl, fd);
        return res.is_ok() ? res.unwrap() : false;
    }

    return true;
}

void Client::Connection::on_disconnect() {}

Result<bool> Client::Connection::connect_next(
    const std::shared_ptr<EventLoop>& event_loop_ptr,
    const std::weak_ptr<Timer>& timer_hdl, int fd) {
    int err;
    socklen_t err_len = sizeof(err);

    std::shared_ptr<Timer> timer = timer_hdl.lock();

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &err_len) < 0) {
        disable_timeout(timer);

        if (m_fail_cb) {
            m_fail_cb();
        }

        return Result<bool>::Err(Error::from_errno(__func__, "getsockopt"));
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

    std::shared_ptr<Connection> next_conn =
        Connection::create(m_logger_context, m_addrs, m_curr_addr->ai_next,
                           m_connect_cb, m_fail_cb);

    event_loop_ptr->add(next_conn, timer ? timer->get_timeout_ms() : -1);

    return Result<bool>::Ok(false);
}

void Client::Connection::disable_timeout(const std::shared_ptr<Timer>& timer) {
    if (timer) {
        m_logger.verbose("[" + m_addr_str + "]: disabled timeout");
        timer->disable();
    }
}

} // namespace tcp