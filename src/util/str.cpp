#include "str.h"

#include <sys/epoll.h>

namespace util {
namespace str {

std::string addr_to_str(addrinfo* addr) {
    if (addr == nullptr) {
        return "???";
    }

    return addr_to_str(addr->ai_addr, addr->ai_addrlen);
}

std::string addr_to_str(sockaddr* sock_addr, socklen_t sock_addr_len) {
    if (sock_addr == nullptr) {
        return "???";
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int result =
        getnameinfo(sock_addr, sock_addr_len, host, sizeof(host), service,
                    sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);

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

std::string epoll_events_to_str(uint32_t events) {
    std::string str = "";

    if (events & EPOLLIN) {
        str += "EPOLLIN ";
    }

    if (events & EPOLLOUT) {
        str += "EPOLLOUT ";
    }

    if (events & EPOLLPRI) {
        str += "EPOLLPRI ";
    }

    if (events & EPOLLHUP) {
        str += "EPULLHUP ";
    }

    if (events & EPOLLRDHUP) {
        str += "EPOLLRDHUP ";
    }

    if (events & EPOLLET) {
        str += "EPOLLET ";
    }

    if (events & EPOLLONESHOT) {
        str += "EPOLLONESHOT ";
    }

    return str.length() > 0 ? str.substr(0, str.length() - 1) : "NONE";
}

} // namespace str
} // namespace util