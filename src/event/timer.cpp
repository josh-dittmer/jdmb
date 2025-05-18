#include "timer.h"

#include "event_loop.h"

#include <sys/epoll.h>
#include <sys/timerfd.h>

Timer::Timer(std::weak_ptr<IOHandle> assoc_hdl, int timeout_ms)
    : IOHandle(EPOLLIN), m_assoc_hdl(assoc_hdl), m_timeout_ms(timeout_ms) {
    int fd = timerfd_create(CLOCK_MONOTONIC, 0);

    if (fd < 0) {
        m_fd_result = Result<int>::Err(Error("failed to create timer fd"));
        return;
    }

    itimerspec timeout;
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_nsec = 0;
    timeout.it_value.tv_sec = m_timeout_ms / 1000;
    timeout.it_value.tv_nsec = m_timeout_ms % 1000;

    if (timerfd_settime(fd, 0, &timeout, nullptr) == -1) {
        m_fd_result = Result<int>::Err(Error("failed to set time on fd"));
        return;
    }

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(fd);
}

Timer::~Timer() {}

bool Timer::on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                    const std::weak_ptr<Timer>&, int fd, uint32_t events) {
    std::shared_ptr<IOHandle> hdl;
    if (!(hdl = m_assoc_hdl.lock())) {
        return false;
    }

    Result<int> epfd_res = event_loop_ptr->m_epfd->get();
    if (!epfd_res.is_ok()) {
        return false;
    }

    event_loop_ptr->epoll_del(epfd_res.unwrap(), hdl);

    return false;
}