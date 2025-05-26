#include "unix.h"

namespace util {
namespace nix {

Result<> set_fd_nonblock(int fd) {
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) {
        return Result<>::Err(Error::from_errno(__func__, "fcntl"));
    }

    return Result<>::Ok(None{});
}

} // namespace nix
} // namespace util