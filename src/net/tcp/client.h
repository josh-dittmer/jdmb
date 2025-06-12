#pragma once

#include "../../event/fd/io_handle.h"
#include "../../log/logger.h"
#include "connection.h"

#include <functional>

#include <netdb.h>
#include <string>
#include <sys/epoll.h>

namespace tcp {

class Client {
  public:
    typedef std::function<void(std::shared_ptr<tcp::Connection>)>
        ConnectCallback;
    typedef std::function<void()> FailCallback;

    class Connection : public tcp::Connection {
        struct Private {
            explicit Private() = default;
        };

      public:
        Connection(Private, std::shared_ptr<LoggerContext> logger_context,
                   addrinfo* addrs, addrinfo* curr_addr,
                   ConnectCallback connect_cb, FailCallback fail_cb);

        ~Connection();

        static std::shared_ptr<Connection>
        create(std::shared_ptr<LoggerContext> logger_context, addrinfo* addrs,
               addrinfo* curr_addr, ConnectCallback connect_cb,
               FailCallback fail_cb) {
            return std::make_shared<Connection>(Private(), logger_context,
                                                addrs, curr_addr, connect_cb,
                                                fail_cb);
        }

      private:
        bool is_connected(const std::shared_ptr<EventLoop>& event_loop_ptr,
                          const std::weak_ptr<Timer>& timer_hdl, int fd,
                          uint32_t events) override;

        void on_disconnect() override;

        Result<bool>
        connect_next(const std::shared_ptr<EventLoop>& event_loop_ptr,
                     const std::weak_ptr<Timer>& timer_hdl, int fd);

        void disable_timeout(const std::shared_ptr<Timer>& timer);

        bool m_connected;

        addrinfo* m_addrs;
        addrinfo* m_curr_addr;

        Client::ConnectCallback m_connect_cb;
        Client::FailCallback m_fail_cb;
    };

    Client(std::shared_ptr<LoggerContext> logger_context,
           const std::string& host, int port)
        : m_logger_context(logger_context),
          m_logger(logger_context->use("TCPClient")), m_host(host),
          m_port(port) {}
    ~Client() {}

    Result<std::shared_ptr<Connection>> get_conn_hdl() const;

    void on_connect(ConnectCallback cb) { m_connect_cb = cb; }
    void on_fail(FailCallback cb) { m_fail_cb = cb; }

    std::string get_host() const { return m_host; }
    int get_port() const { return m_port; }

  private:
    std::shared_ptr<LoggerContext> m_logger_context;
    Logger m_logger;

    std::string m_host;
    int m_port;

    ConnectCallback m_connect_cb;
    FailCallback m_fail_cb;
};

} // namespace tcp