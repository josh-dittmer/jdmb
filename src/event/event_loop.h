#pragma once

#include "../log/logger.h"
#include "fd/epoll_fd.h"
#include "fd/io_handle.h"

#include <memory>
#include <unordered_map>

class EventLoop : public std::enable_shared_from_this<EventLoop> {
    struct Private {
        explicit Private() = default;
    };

  public:
    EventLoop(Private, std::shared_ptr<LoggerContext> logger_context)
        : m_logger(logger_context->use("EventLoop")),
          m_epfd(std::make_unique<EpollFD>()) {}

    ~EventLoop() {}

    static std::shared_ptr<EventLoop>
    create(std::shared_ptr<LoggerContext> logger_context) {
        return std::make_shared<EventLoop>(Private(), logger_context);
    }

    Result<> start();
    Result<> add(const std::shared_ptr<IOHandle>& hdl, int timeout_ms = -1);

    std::shared_ptr<EventLoop> get_ptr() { return shared_from_this(); }

  private:
    struct HandleData {
        std::shared_ptr<IOHandle> m_hdl;
        std::weak_ptr<Timer> m_timer_hdl;
    };

    Result<> epoll_add(int epfd, const std::shared_ptr<IOHandle>& hdl,
                       const std::shared_ptr<Timer>& timer_hdl);
    Result<> epoll_del(int epfd, const std::shared_ptr<IOHandle>& hdl);
    Result<>
    epoll_del(int epfd,
              const std::unordered_map<int, HandleData>::iterator& mit);

    Logger m_logger;

    std::unique_ptr<FD> m_epfd;

    std::unordered_map<int, HandleData> m_hdls;

    friend class Timer;
};