#include "event_loop.h"

#include "fd/epoll_fd.h"
#include "timer.h"

#include <sys/epoll.h>
#include <unistd.h>

Result<> EventLoop::start() {
    Result<int> epfd_res = m_epfd->get();
    if (!epfd_res.is_ok()) {
        return Result<>::Err(Error(__func__, epfd_res));
    }

    int epfd = epfd_res.unwrap();

    static const int MAX_EVENTS = 32;
    epoll_event events[MAX_EVENTS];

    m_logger.verbose("start(): started");

    while (!m_hdls.empty()) {
        int num_fds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < num_fds; i++) {
            int fd = events[i].data.fd;
            uint32_t evnts = events[i].events;

            auto mit = m_hdls.find(fd);
            if (mit == m_hdls.end()) {
                m_logger.debug("start(): [" + std::to_string(fd) +
                               "] not in epoll list");
                continue;
            }

            std::shared_ptr<IOHandle> hdl = mit->second.m_hdl;
            std::weak_ptr<Timer> timer_hdl = mit->second.m_timer_hdl;

            if (!hdl->on_data(get_ptr(), timer_hdl, fd, evnts)) {
                epoll_del(epfd, mit);
            }
        }
    }

    m_logger.verbose("start(): exited");

    return Result<>::Ok(None{});
}

Result<> EventLoop::add(const std::shared_ptr<IOHandle>& io_hdl,
                        int timeout_ms) {
    Result<int> epfd_res = m_epfd->get();
    if (!epfd_res.is_ok()) {
        return Result<>::Err(Error(__func__, epfd_res));
    }

    int epfd = epfd_res.unwrap();

    std::shared_ptr<Timer> timer_hdl = nullptr;
    if (timeout_ms > 0) {
        timer_hdl = std::make_shared<Timer>(
            [=](std::shared_ptr<EventLoop> event_loop) {
                event_loop->epoll_del(epfd, io_hdl);
            },
            timeout_ms);

        Result<> timer_res = add(timer_hdl, -1);
        if (!timer_res.is_ok()) {
            m_logger.warn("Set timeout failed: " + timer_res.unwrap_err());
        }
    }

    return epoll_add(epfd, io_hdl, timer_hdl);
}

Result<> EventLoop::epoll_add(int epfd, const std::shared_ptr<IOHandle>& hdl,
                              const std::shared_ptr<Timer>& timer_hdl) {
    Result<int> hdl_res = hdl->get();
    if (!hdl_res.is_ok()) {
        return Result<>::Err(Error(__func__, hdl_res));
    }

    int fd = hdl_res.unwrap();

    epoll_event ev;
    ev.events = hdl->get_enabled_events();
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        return Result<>::Err(Error::from_errno(__func__, "epoll_ctl"));
    }

    m_logger.verbose("epoll_ctl_add(): +[" + std::to_string(fd) + "]");

    HandleData hdl_data = {.m_hdl = hdl, .m_timer_hdl = timer_hdl};
    m_hdls.insert(std::make_pair(fd, hdl_data));

    return Result<>::Ok(None{});
}

Result<> EventLoop::epoll_del(int epfd, const std::shared_ptr<IOHandle>& hdl) {
    Result<int> hdl_res = hdl->get();
    if (!hdl_res.is_ok()) {
        return Result<>::Err(Error(__func__, hdl_res));
    }

    int fd = hdl_res.unwrap();

    auto mit = m_hdls.find(fd);
    if (mit == m_hdls.end()) {
        return Result<>::Err(Error(__func__, "handle not found"));
    }

    return epoll_del(epfd, mit);
}

Result<>
EventLoop::epoll_del(int epfd,
                     const std::unordered_map<int, HandleData>::iterator& mit) {
    int fd = mit->first;

    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        return Result<>::Err(Error::from_errno(__func__, "epoll_ctl"));
    }

    m_hdls.erase(mit);

    m_logger.verbose("epoll_ctl_del(): -[" + std::to_string(fd) + "]");

    return Result<>::Ok(None{});
}
