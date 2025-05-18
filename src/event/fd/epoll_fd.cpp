#include "epoll_fd.h"

#include <sys/epoll.h>

EpollFD::EpollFD() : FD() {
    int epfd = epoll_create(1);
    if (epfd == -1) {
        m_fd_result = Result<int>::Err(Error("failed to create epoll fd"));
    }

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(epfd);
}