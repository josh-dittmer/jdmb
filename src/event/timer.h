#pragma once

#include "fd/io_handle.h"

#include <functional>

class Timer : public IOHandle {
  public:
    typedef std::function<void(std::shared_ptr<EventLoop>)> TimeoutCallback;

    Timer(TimeoutCallback cb, int timeout_ms);
    ~Timer();

    void disable() { m_timeout_cb = nullptr; }

    int get_timeout_ms() { return m_timeout_ms; }

  private:
    bool on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                 const std::weak_ptr<Timer>&, int fd, uint32_t events) override;

    TimeoutCallback m_timeout_cb;

    int m_timeout_ms;
};