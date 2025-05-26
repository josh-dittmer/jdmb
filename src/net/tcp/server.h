#pragma once

#include "../../log/logger.h"
#include "connection.h"

#include <functional>

#include <arpa/inet.h>
#include <sys/epoll.h>

namespace tcp {

class Server : public IOHandle {
    struct Private {
        explicit Private() = default;
    };

  public:
    typedef std::function<void(std::shared_ptr<tcp::Connection>)>
        ConnectCallback;

    class Connection : public tcp::Connection {
        struct Private {
            explicit Private() = default;
        };

      public:
        Connection(Private, std::shared_ptr<LoggerContext> logger_context,
                   int fd, sockaddr_in client_addr, ConnectCallback connect_cb);

        ~Connection();

        static Result<std::shared_ptr<Connection>>
        create(std::shared_ptr<LoggerContext> logger_context, int server_fd,
               ConnectCallback connect_cb);

      private:
        bool is_connected(const std::shared_ptr<EventLoop>& event_loop_ptr,
                          const std::weak_ptr<Timer>& timer_hdl, int fd,
                          uint32_t events) override;
    };

    Server(Private, std::shared_ptr<LoggerContext> logger_context, int port,
           int max_queue_len);

    ~Server();

    static std::shared_ptr<Server>
    create(std::shared_ptr<LoggerContext> logger_context, int port,
           int max_queue_len) {
        return std::make_shared<Server>(Private(), logger_context, port,
                                        max_queue_len);
    }

    void on_connect(ConnectCallback cb) { m_connect_cb = cb; }

    int get_port() const { return m_port; }

  private:
    bool on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                 const std::weak_ptr<Timer>& timer_hdl, int fd,
                 uint32_t events) override;

    void accept(const std::shared_ptr<EventLoop>& event_loop_ptr, int fd);

    std::shared_ptr<LoggerContext> m_logger_context;
    Logger m_logger;

    int m_port;

    ConnectCallback m_connect_cb;
};

} // namespace tcp