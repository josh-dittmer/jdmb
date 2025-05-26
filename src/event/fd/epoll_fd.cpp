#include "epoll_fd.h"

#include <sys/epoll.h>

EpollFD::EpollFD() : FD() {
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        m_fd_result =
            Result<int>::Err(Error::from_errno(__func__, "epoll_create"));
        return;
    }

    _OPEN_COUNT++;
    m_fd_result = Result<int>::Ok(epfd);
}