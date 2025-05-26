#pragma once

#include <netdb.h>
#include <string>
#include <vector>

namespace util {
namespace str {

extern std::string addr_to_str(addrinfo* addr_info);
extern std::string addr_to_str(sockaddr* sock_addr, socklen_t sock_addr_len);

extern std::string bytes_to_str(const std::vector<uint8_t> buf);
extern std::vector<uint8_t> str_to_bytes(const std::string& str);

extern std::string epoll_events_to_str(uint32_t events);

} // namespace str
} // namespace util