#include "fd.h"

#include <unistd.h>

int FD::_OPEN_COUNT = 0;
int FD::_CLOSE_COUNT = 0;

FD::FD() : m_fd_result(Result<int>::Err(Error("fd not opened"))) {}

FD::~FD() {
    if (m_fd_result.is_ok()) {
        ::close(m_fd_result.unwrap());
        _CLOSE_COUNT++;
    }
}