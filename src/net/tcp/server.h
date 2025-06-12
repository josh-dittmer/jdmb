#pragma once

#include "../../log/logger.h"
#include "connection.h"

#include <functional>

#include <arpa/inet.h>
#include <sys/epoll.h>

namespace tcp {

class Server : public IOHandle, public std::enable_shared_from_this<Server> {
    struct Private {
        explicit Private() = default;
    };

  public:
    typedef std::function<void(std::shared_ptr<tcp::Connection>)>
        ConnectCallback;
    typedef std::function<void(std::shared_ptr<tcp::Connection>)>
        DisconnectCallback;

    class Connection : public tcp::Connection {
        struct Private {
            explicit Private() = default;
        };

      public:
        Connection(Private, std::shared_ptr<LoggerContext> logger_context,
                   int fd, sockaddr_storage client_addr,
                   Server::DisconnectCallback server_disconnect_cb);

        ~Connection();

        static Result<std::shared_ptr<Connection>>
        create(std::shared_ptr<LoggerContext> logger_context,
               std::shared_ptr<FD> server_fd,
               Server::DisconnectCallback server_disconnect_cb);

      private:
        bool is_connected(const std::shared_ptr<EventLoop>& event_loop_ptr,
                          const std::weak_ptr<Timer>& timer_hdl, int fd,
                          uint32_t events) override;

        void on_disconnect() override;

        Server::DisconnectCallback m_server_disconnect_cb;
    };

  public:
    Server(Private, std::shared_ptr<LoggerContext> logger_context,
           const std::string& host, int port, int max_queue_len);

    ~Server();

    static std::shared_ptr<Server>
    create(std::shared_ptr<LoggerContext> logger_context,
           const std::string& host, int port, int max_queue_len) {
        return std::make_shared<Server>(Private(), logger_context, host, port,
                                        max_queue_len);
    }

    void on_connect(ConnectCallback cb) { m_connect_cb = cb; }
    void on_disconnect(DisconnectCallback cb) { m_disconnect_cb = cb; }

    int get_port() const { return m_port; }

  protected:
    Logger m_logger;

  private:
    bool on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                 const std::weak_ptr<Timer>& timer_hdl, int fd,
                 uint32_t events) override;

    void accept(const std::shared_ptr<EventLoop>& event_loop_ptr, int fd);

    std::shared_ptr<LoggerContext> m_logger_context;

    std::string m_host;
    int m_port;

    ConnectCallback m_connect_cb;
    DisconnectCallback m_disconnect_cb;
};

} // namespace tcp