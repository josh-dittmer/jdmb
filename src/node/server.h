#pragma once

#include "../event/event_loop.h"
#include "../net/tcp/server.h"

#include <map>
#include <string>

namespace node {

class Server {
    struct Private {
        explicit Private() = default;
    };

  public:
    class Connection : public std::enable_shared_from_this<Connection> {
        struct Private {
            explicit Private() = default;
        };

      public:
        Connection(Private, std::shared_ptr<LoggerContext> logger_context)
            : m_logger(logger_context->use("NodeConnection")),
              m_bytes_needed(0) {}
        ~Connection() {}

        static std::shared_ptr<Connection>
        create(std::shared_ptr<LoggerContext> logger_context) {
            return std::make_shared<Connection>(Private(), logger_context);
        }

        void on_connect(std::shared_ptr<tcp::Connection>);
        bool on_message(std::shared_ptr<tcp::Connection> conn,
                        const std::vector<uint8_t>& data);
        void on_disconnect(std::shared_ptr<tcp::Connection> conn);

      private:
        Logger m_logger;

        std::size_t m_bytes_needed;
        std::vector<uint8_t> m_curr_packet;
    };

    Server(Private, std::shared_ptr<LoggerContext> logger_context,
           const std::string& host, int port, int max_queue_len);

    ~Server() {}

    static Result<std::shared_ptr<Server>>
    create(std::shared_ptr<EventLoop> event_loop,
           std::shared_ptr<LoggerContext> logger_context,
           const std::string& host, int port, int max_queue_len);

  private:
    std::shared_ptr<LoggerContext> m_logger_context;
    Logger m_logger;

    std::shared_ptr<tcp::Server> m_tcp_server;

    std::map<std::shared_ptr<tcp::Connection>, std::shared_ptr<Connection>,
             std::owner_less<std::shared_ptr<tcp::Connection>>>
        m_connections;

    void on_connect(std::shared_ptr<tcp::Connection> tcp_conn);
    void on_disconnect(std::shared_ptr<tcp::Connection> tcp_conn);
};

} // namespace node