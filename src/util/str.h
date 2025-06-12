#pragma once

#include "result.h"

#include <netdb.h>
#include <string>
#include <vector>

namespace util {
namespace str {

extern std::string to_lower(std::string str);
extern int to_int(const std::string& str);
extern std::vector<std::string> split(std::string str, char delim);

extern Result<std::string> addr_to_str(addrinfo* addr_info);
extern Result<std::string> addr_to_str(sockaddr* sock_addr,
                                       socklen_t sock_addr_len);

extern std::string bytes_to_str(const std::vector<uint8_t> buf);
extern std::vector<uint8_t> str_to_bytes(const std::string& str);

extern std::string epoll_events_to_str(uint32_t events);

} // namespace str
} // namespace util