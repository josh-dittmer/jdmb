#pragma once

#include <fcntl.h>

#include "result.h"

namespace util {
namespace nix {

extern Result<> set_fd_nonblock(int fd);

}
} // namespace util