#include "debug.h"

namespace util {
namespace debug {

Result<> send_test_buf(std::shared_ptr<tcp::Connection> conn, std::size_t size,
                       std::size_t num) {
    std::vector<uint8_t> test_buf;
    test_buf.reserve(size);

    for (std::size_t i = 0; i < size; i++) {
        test_buf.push_back((uint8_t)'A');
    }

    for (std::size_t i = 0; i < num; i++) {
        Result<int> s_res = conn->send(test_buf);
        if (!s_res.is_ok()) {
            return Result<>::Err(Error(__func__, s_res));
        }
    }

    return Result<>::Ok(None{});
}

} // namespace debug
}; // namespace util