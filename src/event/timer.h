#pragma once

#include "fd/io_handle.h"

class Timer : public IOHandle {
  public:
    Timer(std::weak_ptr<IOHandle> assoc_hdl, int timeout_ms);
    ~Timer();

    void disable() { m_assoc_hdl.reset(); }

    int get_timeout_ms() { return m_timeout_ms; }

  private:
    bool on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                 const std::weak_ptr<Timer>&, int fd, uint32_t events) override;

    std::weak_ptr<IOHandle> m_assoc_hdl;
    int m_timeout_ms;
};