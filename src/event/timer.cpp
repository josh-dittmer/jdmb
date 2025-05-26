#include "timer.h"

#include "../util/unix.h"
#include "event_loop.h"

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

Timer::Timer(TimeoutCallback cb, int timeout_ms)
    : IOHandle(EPOLLIN), m_timeout_cb(cb), m_timeout_ms(timeout_ms) {
    int fd = timerfd_create(CLOCK_MONOTONIC, 0);

    if (fd < 0) {
        m_fd_result =
            Result<int>::Err(Error::from_errno(__func__, "timerfd_create"));
        return;
    }

    Result<> nb_res = util::nix::set_fd_nonblock(fd);
    if (!nb_res.is_ok()) {
        ::close(fd);
        m_fd_result = Result<int>::Err(Error(__func__, nb_res));
        return;
    }

    itimerspec timeout;
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_nsec = 0;
    timeout.it_value.tv_sec = m_timeout_ms / 1000;
    timeout.it_value.tv_nsec = m_timeout_ms % 1000;

    if (timerfd_settime(fd, 0, &timeout, nullptr) == -1) {
        m_fd_result =
            Result<int>::Err(Error::from_errno(__func__, "timerfd_settime"));
        return;
    }

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(fd);
}

Timer::~Timer() {}

bool Timer::on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                    const std::weak_ptr<Timer>&, int fd, uint32_t events) {
    if (!m_timeout_cb) {
        return false;
    }

    m_timeout_cb(event_loop_ptr);

    return false;
}