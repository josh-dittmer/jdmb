#pragma once

#include "../../event/fd/io_handle.h"
#include "../../log/logger.h"

#include <functional>
#include <queue>

namespace tcp {

class Connection : public IOHandle,
                   public std::enable_shared_from_this<Connection> {
  public:
    typedef std::function<bool(std::shared_ptr<Connection>,
                               const std::vector<uint8_t>&)>
        MessageCallback;

    typedef std::function<void(std::shared_ptr<Connection>)> DisconnectCallback;

  protected:
    Connection(uint32_t events, std::shared_ptr<LoggerContext> logger_context,
               const std::string& logger_context_str)
        : IOHandle(events), m_logger_context(logger_context),
          m_logger(logger_context->use(logger_context_str)), m_send_ready(true),
          m_flushing(false) {}

  public:
    virtual ~Connection() = default;

    Result<int> send(const std::vector<uint8_t>& data);

    void on_message(MessageCallback cb) { m_message_cb = cb; }
    void on_disconnect(DisconnectCallback cb) { m_disconnect_cb = cb; }

    std::string get_addr_str() { return m_addr_str; }

    std::shared_ptr<Connection> get_ptr() { return shared_from_this(); }

  protected:
    std::shared_ptr<LoggerContext> m_logger_context;
    Logger m_logger;

    std::string m_addr_str;

  private:
    bool on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                 const std::weak_ptr<Timer>& timer_hdl, int fd,
                 uint32_t events) override;

    virtual bool is_connected(const std::shared_ptr<EventLoop>& event_loop_ptr,
                              const std::weak_ptr<Timer>& timer_hdl, int fd,
                              uint32_t events) = 0;

    virtual void on_disconnect() = 0;

    Result<bool> recv(int fd, std::vector<uint8_t>& buf, int max);
    Result<int> flush_send_queue();

    bool m_send_ready;
    std::queue<std::vector<uint8_t>> m_send_queue;

    bool m_flushing;

    MessageCallback m_message_cb;
    DisconnectCallback m_disconnect_cb;
};

} // namespace tcp