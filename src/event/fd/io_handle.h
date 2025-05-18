#pragma once

#include "fd.h"

#include "../../util/result.h"

#include <cstdint>
#include <memory>

class EventLoop;
class Timer;

class IOHandle : public FD {
  protected:
    IOHandle(uint32_t enabled_events) : m_enabled_events(enabled_events) {}

  public:
    ~IOHandle() {}

    virtual bool on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                         const std::weak_ptr<Timer>& timer_hdl, int fd,
                         uint32_t events) = 0;

    uint32_t get_enabled_events() { return m_enabled_events; }

  private:
    uint32_t m_enabled_events;
};