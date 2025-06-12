#include "server.h"

#include "../../event/event_loop.h"
#include "../../util/str.h"
#include "../../util/unix.h"

#include <sys/socket.h>
#include <unistd.h>

namespace tcp {

Server::Server(Private, std::shared_ptr<LoggerContext> logger_context,
               const std::string& host, int port, int max_queue_len)
    : IOHandle(EPOLLET | EPOLLIN | EPOLLOUT), m_logger_context(logger_context),
      m_logger(logger_context->use("TCPServer")), m_host(host), m_port(port) {
    addrinfo* addrs;

    addrinfo hints = {.ai_flags = AI_PASSIVE,
                      .ai_family = AF_UNSPEC,
                      .ai_socktype = SOCK_STREAM};

    int err = getaddrinfo(m_host.c_str(), std::to_string(m_port).c_str(),
                          &hints, &addrs);

    if (err != 0) {
        m_fd_result =
            Result<int>::Err(Error::from_ai_err(__func__, "getaddrinfo", err));
        return;
    }

    addrinfo* used_addr = nullptr;
    int fd = -1;

    for (addrinfo* addr = addrs; addr != nullptr; addr = addr->ai_next) {
        fd = socket(addr->ai_family, addr->ai_socktype | SOCK_NONBLOCK,
                    addr->ai_protocol);
        if (fd < 0) {
            Error err = Error::from_errno(__func__, "socket");
            m_logger.verbose("socket(): failed, trying next..." + err);
            continue;
        }

        const int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
            ::close(fd);
            Error err = Error::from_errno(__func__, "setsockopt");
            m_logger.verbose("setsockopt(): failed, trying next..." + err);
            continue;
        }

        if (bind(fd, addr->ai_addr, addr->ai_addrlen) == 0) {
            used_addr = addr;
            break;
        }

        m_logger.verbose("bind(): failed, trying next..." + err);

        ::close(fd);
        fd = -1;
    }

    if (fd == -1) {
        m_fd_result = Result<int>::Err(Error::from_errno(__func__, "bind"));
        return;
    }

    if (listen(fd, max_queue_len) < 0) {
        ::close(fd);
        m_fd_result = Result<int>::Err(Error::from_errno(__func__, "listen"));
        return;
    }

    std::string addr = "???";
    Result<std::string> addr_res = util::str::addr_to_str(used_addr);
    if (!addr_res.is_ok()) {
        m_logger.warn("Failed to convert address to string: " +
                      addr_res.unwrap_err());
    } else {
        addr = addr_res.unwrap();
    }

    m_logger.verbose("listening on " + addr);

    freeaddrinfo(addrs);

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(fd);
}

Server::~Server() {}

bool Server::on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                     const std::weak_ptr<Timer>& timer_hdl, int fd,
                     uint32_t events) {
    if (events & EPOLLIN) {
        accept(event_loop_ptr, fd);
    }

    return true;
}

void Server::accept(const std::shared_ptr<EventLoop>& event_loop_ptr, int fd) {
    int total = 0;
    while (true) {
        Result<std::shared_ptr<Connection>> conn_res = Connection::create(
            m_logger_context, shared_from_this(), m_disconnect_cb);

        if (!conn_res.is_ok()) {
            m_logger.debug("accept(): failed: " + conn_res.unwrap_err());
            continue;
        }

        std::shared_ptr<Connection> conn = conn_res.unwrap();
        if (!conn) {
            break;
        }

        Result<> add_res = event_loop_ptr->add(conn);
        if (!add_res.is_ok()) {
            m_logger.debug("accept(): event loop add failed: " +
                           add_res.unwrap_err());
            continue;
        }

        total++;

        if (m_connect_cb) {
            m_connect_cb(conn);
        }
    }

    m_logger.verbose("accept(): " + std::to_string(total) +
                     " connections accepted");
}

Server::Connection::Connection(Private,
                               std::shared_ptr<LoggerContext> logger_context,
                               int fd, sockaddr_storage client_addr,
                               DisconnectCallback server_disconnect_cb)
    : tcp::Connection(EPOLLET | EPOLLIN | EPOLLOUT, logger_context,
                      "STCPConnection") {
    if (fd < 0) {
        m_fd_result = Result<int>::Err(Error(__func__, "bad fd"));
        return;
    }

    Result<> nb_res = util::nix::set_fd_nonblock(fd);
    if (!nb_res.is_ok()) {
        ::close(fd);
        m_fd_result = Result<int>::Err(Error(__func__, nb_res));
        return;
    }

    socklen_t client_addr_len = sizeof(client_addr);

    m_addr_str = "???";
    Result<std::string> addr_res =
        util::str::addr_to_str((sockaddr*)&client_addr, client_addr_len);
    if (!addr_res.is_ok()) {
        m_logger.warn("Failed to convert address to string: " +
                      addr_res.unwrap_err());
    } else {
        m_addr_str = addr_res.unwrap();
    }

    m_logger.verbose("[" + m_addr_str + "]: connection established");

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(fd);
}

Server::Connection::~Connection() {}

Result<std::shared_ptr<Server::Connection>>
Server::Connection::create(std::shared_ptr<LoggerContext> logger_context,
                           std::shared_ptr<FD> server_fd,
                           Server::DisconnectCallback server_disconnect_cb) {
    Result<int> server_fd_res = server_fd->get();
    if (!server_fd_res.is_ok()) {
        return Result<std::shared_ptr<Server::Connection>>::Err(
            Error(__func__, server_fd_res));
    }

    sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int fd = ::accept(server_fd_res.unwrap(), (sockaddr*)&client_addr,
                      &client_addr_len);
    if (fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return Result<std::shared_ptr<Server::Connection>>::Ok(nullptr);
        }

        return Result<std::shared_ptr<Server::Connection>>::Err(
            Error::from_errno(__func__, "accept"));
    }

    std::shared_ptr<Connection> conn = std::make_shared<Connection>(
        Private(), logger_context, fd, client_addr, server_disconnect_cb);

    return Result<std::shared_ptr<Connection>>::Ok(conn);
}

bool Server::Connection::is_connected(
    const std::shared_ptr<EventLoop>& event_loop_ptr,
    const std::weak_ptr<Timer>& timer_hdl, int fd, uint32_t events) {
    return true;
}

void Server::Connection::on_disconnect() {
    if (m_server_disconnect_cb) {
        m_server_disconnect_cb(get_ptr());
    }
}

} // namespace tcp