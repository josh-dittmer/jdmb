#include "server.h"

#include "../../event/event_loop.h"
#include "../../util/str.h"
#include "../../util/unix.h"

#include <sys/socket.h>
#include <unistd.h>

namespace tcp {

Server::Server(Private, std::shared_ptr<LoggerContext> logger_context, int port,
               int max_queue_len)
    : IOHandle(EPOLLET | EPOLLIN | EPOLLOUT), m_logger_context(logger_context),
      m_logger(logger_context->use("TCPServer")), m_port(port) {

    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (fd < 0) {
        m_fd_result = Result<int>::Err(Error::from_errno(__func__, "socket"));
        return;
    }

    const int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
        ::close(fd);
        m_fd_result =
            Result<int>::Err(Error::from_errno(__func__, "setsockopt"));
        return;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ::close(fd);
        m_fd_result = Result<int>::Err(Error::from_errno(__func__, "bind"));
        return;
    }

    if (listen(fd, max_queue_len) < 0) {
        ::close(fd);
        m_fd_result = Result<int>::Err(Error::from_errno(__func__, "listen"));
        return;
    }

    m_logger.verbose("listening on port " + std::to_string(m_port));

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
        Result<std::shared_ptr<Connection>> conn_res =
            Connection::create(m_logger_context, fd, m_connect_cb);

        if (!conn_res.is_ok()) {
            m_logger.debug("accept(): failed: " + conn_res.unwrap_err());
            continue;
        }

        std::shared_ptr<Connection> conn = conn_res.unwrap();
        if (!conn) {
            break;
        }

        total++;
        event_loop_ptr->add(conn);
    }

    m_logger.verbose("accept(): " + std::to_string(total) +
                     " connections accepted");
}

Server::Connection::Connection(Private,
                               std::shared_ptr<LoggerContext> logger_context,
                               int fd, sockaddr_in client_addr,
                               ConnectCallback connect_cb)
    : tcp::Connection(EPOLLET | EPOLLIN | EPOLLOUT, logger_context,
                      "TCPServerConn") {
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

    m_addr_str =
        util::str::addr_to_str((sockaddr*)&client_addr, client_addr_len);

    m_logger.verbose("[" + m_addr_str + "]: connection established");

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(fd);
}

Server::Connection::~Connection() {}

Result<std::shared_ptr<Server::Connection>>
Server::Connection::create(std::shared_ptr<LoggerContext> logger_context,
                           int server_fd, ConnectCallback connect_cb) {
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int fd = ::accept(server_fd, (sockaddr*)&client_addr, &client_addr_len);
    if (fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return Result<std::shared_ptr<Server::Connection>>::Ok(nullptr);
        }

        return Result<std::shared_ptr<Server::Connection>>::Err(
            Error::from_errno(__func__, "accept"));
    }

    std::shared_ptr<Connection> conn = std::make_shared<Connection>(
        Private(), logger_context, fd, client_addr, connect_cb);

    if (connect_cb) {
        connect_cb(conn);
    }

    return Result<std::shared_ptr<Connection>>::Ok(conn);
}

bool Server::Connection::is_connected(
    const std::shared_ptr<EventLoop>& event_loop_ptr,
    const std::weak_ptr<Timer>& timer_hdl, int fd, uint32_t events) {
    return true;
}

} // namespace tcp