#include "str.h"

namespace util {
namespace str {

std::string addr_to_str(addrinfo* addr) {
    if (addr == nullptr) {
        return "???";
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int result =
        getnameinfo(addr->ai_addr, addr->ai_addrlen, host, sizeof(host),
                    service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);

    if (result != 0) {
        return "???";
    }

    return std::string(host) + ":" + std::string(service);
}

std::string bytes_to_str(const std::vector<uint8_t> buf) {
    return std::string(buf.begin(), buf.end());
}

std::vector<uint8_t> str_to_bytes(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

} // namespace str
} // namespace util