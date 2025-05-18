#pragma once

#include "../../event/fd/io_handle.h"
#include "../../log/logger.h"

#include <functional>
#include <netdb.h>
#include <string>
#include <sys/epoll.h>

namespace tcp {

class ClientConnection;

class Client {
  public:
    typedef std::function<void(std::shared_ptr<ClientConnection>)>
        ConnectCallback;

    typedef std::function<void()> FailCallback;

    Client(std::shared_ptr<LoggerContext> logger_context,
           const std::string& host, int port)
        : m_logger_context(logger_context),
          m_logger(logger_context->use("TCPClient")), m_host(host),
          m_port(port) {}
    ~Client() {}

    Result<std::shared_ptr<ClientConnection>> get_conn_hdl();

    void on_connect(ConnectCallback cb) { m_connect_cb = cb; }
    void on_fail(FailCallback cb) { m_fail_cb = cb; }

    std::string get_host() { return m_host; }
    int get_port() { return m_port; }

  private:
    std::shared_ptr<LoggerContext> m_logger_context;
    Logger m_logger;

    std::string m_host;
    int m_port;

    Client::ConnectCallback m_connect_cb;
    Client::FailCallback m_fail_cb;
};

class ClientConnection : public IOHandle,
                         public std::enable_shared_from_this<ClientConnection> {
    struct Private {
        explicit Private() = default;
    };

  public:
    typedef std::function<bool(std::shared_ptr<ClientConnection>,
                               const std::vector<uint8_t>&)>
        MessageCallback;
    typedef std::function<void(std::shared_ptr<ClientConnection>)>
        DisconnectCallback;

    ClientConnection(Private, std::shared_ptr<LoggerContext> logger_context,
                     addrinfo* addrs, addrinfo* curr_addr,
                     Client::ConnectCallback connect_cb,
                     Client::FailCallback fail_cb);

    ~ClientConnection();

    static std::shared_ptr<ClientConnection>
    create(std::shared_ptr<LoggerContext> logger_context, addrinfo* addrs,
           addrinfo* curr_addr, Client::ConnectCallback connect_cb,
           Client::FailCallback fail_cb) {
        return std::make_shared<ClientConnection>(
            Private(), logger_context, addrs, curr_addr, connect_cb, fail_cb);
    }

    Result<int> send(const std::vector<uint8_t>& data);

    void on_message(MessageCallback cb) { m_message_cb = cb; }
    void on_disconnect(DisconnectCallback cb) { m_disconnect_cb = cb; }

    std::string get_addr_str() { return m_addr_str; }

    std::shared_ptr<ClientConnection> get_ptr() { return shared_from_this(); }

  private:
    bool on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                 const std::weak_ptr<Timer>& timer_hdl, int fd,
                 uint32_t events) override;

    Result<bool> connect_next(const std::shared_ptr<EventLoop>& event_loop_ptr,
                              const std::weak_ptr<Timer>& timer_hdl, int fd);

    void disable_timeout(const std::shared_ptr<Timer>& timer);

    Result<std::vector<uint8_t>> recv(int fd);

    std::shared_ptr<LoggerContext> m_logger_context;
    Logger m_logger;

    bool m_connected;

    addrinfo* m_addrs;
    addrinfo* m_curr_addr;

    std::string m_addr_str;

    Client::ConnectCallback m_connect_cb;
    Client::FailCallback m_fail_cb;

    MessageCallback m_message_cb;
    DisconnectCallback m_disconnect_cb;
};

} // namespace tcp