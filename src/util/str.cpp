#include "str.h"

#include <algorithm>

#include <sys/epoll.h>

namespace util {
namespace str {

std::string to_lower(std::string str) {
    std::transform(
        str.begin(), str.end(), str.begin(),
        [](unsigned char c) -> unsigned char { return std::tolower(c); });

    return str;
}

int to_int(const std::string& str) {
    try {
        return std::stoi(str);
    } catch (std::exception& e) {
        return -1;
    }
}

std::vector<std::string> split(std::string str, char delim) {
    std::size_t pos = 0;
    std::vector<std::string> tokens;

    while ((pos = str.find(delim)) != std::string::npos) {
        tokens.push_back(str.substr(0, pos));
        str.erase(0, pos + 1);
    }

    tokens.push_back(str);

    return tokens;
}

Result<std::string> addr_to_str(addrinfo* addr) {
    if (addr == nullptr) {
        return Result<std::string>::Err(Error(__func__, "bad address"));
    }

    return addr_to_str(addr->ai_addr, addr->ai_addrlen);
}

Result<std::string> addr_to_str(sockaddr* sock_addr, socklen_t sock_addr_len) {
    if (sock_addr == nullptr) {
        return Result<std::string>::Err(Error(__func__, "bad address"));
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int err = getnameinfo(sock_addr, sock_addr_len, host, sizeof(host), service,
                          sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);

    if (err != 0) {
        return Result<std::string>::Err(
            Error::from_ai_err(__func__, "getnameinfo", err));
    }

    return Result<std::string>::Ok(std::string(host) + ":" +
                                   std::string(service));
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