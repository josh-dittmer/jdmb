#pragma once

#include "../net/tcp/connection.h"

namespace util {
namespace debug {

extern Result<> send_test_buf(std::shared_ptr<tcp::Connection> conn,
                              std::size_t size, std::size_t num);

}
} // namespace util