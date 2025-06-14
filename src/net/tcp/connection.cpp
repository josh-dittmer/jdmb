#include "connection.h"

#include "../../event/event_loop.h"
#include "../../event/timer.h"
#include "../../util/str.h"

#include <arpa/inet.h>
#include <sys/epoll.h>

namespace tcp {

Result<int> Connection::send(const std::vector<uint8_t>& data) {
    if (!m_fd_result.is_ok()) {
        return Result<int>::Err(Error(__func__, m_fd_result));
    }

    if (!m_send_ready) {
        m_send_queue.emplace(data);
        m_logger.verbose("[" + m_addr_str + "]: buffer full: queued " +
                         std::to_string(data.size()) + " bytes");
        return Result<int>::Ok(0);
    }

    int fd = m_fd_result.unwrap();

    int total_sent = 0;
    while (total_sent < data.size()) {
        int bytes_sent =
            ::send(fd, &data[total_sent], data.size() - total_sent, 0);

        if (bytes_sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (!m_flushing) {
                    std::vector<uint8_t> partial_data(data.begin() + total_sent,
                                                      data.end());
                    m_send_queue.emplace(partial_data);

                    m_logger.verbose(
                        "[" + m_addr_str + "]: buffer full: sent " +
                        std::to_string(total_sent) + "/" +
                        std::to_string(data.size()) + " bytes, queued " +
                        std::to_string(partial_data.size()) + " bytes");
                }

                m_send_ready = false;

                break;
            }

            m_logger.debug("[" + m_addr_str +
                           "]: error: " + std::string(strerror(errno)));
            return Result<int>::Err(Error::from_errno(__func__, "send"));
        }

        total_sent += bytes_sent;

        m_logger.verbose("[" + m_addr_str + "]: sent " +
                         std::to_string(total_sent) + "/" +
                         std::to_string(data.size()) + " bytes");
    }

    return Result<int>::Ok(total_sent);
}

Result<bool> Connection::recv(int fd, std::vector<uint8_t>& buf, int max) {
    buf.clear();
    static const int CHUNK_SIZE = 4096;

    uint8_t chunk[CHUNK_SIZE];
    bool again = false;

    while (true) {
        int size = buf.size() > max ? max - buf.size() : 4096;

        int bytes_received = ::recv(fd, chunk, size, 0);
        if (bytes_received > 0) {
            buf.insert(buf.end(), chunk, chunk + bytes_received);
            if (buf.size() >= max) {
                again = true;
                break;
            }
        }

        else if (bytes_received == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }

            return Result<bool>::Err(Error::from_errno(__func__, "recv"));
        }

        else {
            return Result<bool>::Err(Error(__func__, "connection closed"));
        }
    }

    m_logger.verbose("[" + m_addr_str + "]: received " +
                     std::to_string(buf.size()) + " bytes");
    return Result<bool>::Ok(again);
}

Result<int> Connection::flush_send_queue() {
    m_send_ready = true;
    m_flushing = true;

    int total_msgs_sent = 0;
    while (!m_send_queue.empty() && m_send_ready) {
        std::vector<uint8_t> data = m_send_queue.front();

        Result<int> send_res = send(data);
        if (!send_res.is_ok()) {
            return Result<int>::Err(Error(__func__, send_res));
        }

        int total_sent = send_res.unwrap();
        if (total_sent < data.size()) {
            data.erase(data.begin(), data.begin() + total_sent);
            m_send_queue.front() = data;

            m_logger.verbose("[" + m_addr_str + "]: buffer full, sent " +
                             std::to_string(total_sent) + " bytes, requeued " +
                             std::to_string(data.size()) + " bytes and " +
                             std::to_string(m_send_queue.size() - 1) +
                             " additional messages");

            break;
        }

        total_msgs_sent++;
        m_send_queue.pop();
    }

    m_flushing = false;

    m_logger.verbose("[" + m_addr_str + "]: flushed send queue");

    return Result<int>::Ok(0);
}

bool Connection::on_data(const std::shared_ptr<EventLoop>& event_loop_ptr,
                         const std::weak_ptr<Timer>& timer_hdl, int fd,
                         uint32_t events) {
    m_logger.verbose("[" + m_addr_str + "]: events [" +
                     util::str::epoll_events_to_str(events) + "]");

    bool should_close = !is_connected(event_loop_ptr, timer_hdl, fd, events);

    if (!should_close && (events & EPOLLOUT)) {
        flush_send_queue();
    }

    if (!should_close && (events & EPOLLHUP)) {
        m_logger.verbose("[" + m_addr_str +
                         "]: connection closed by remote peer (HUP)");

        should_close = true;
    }

    if (!should_close && (events & EPOLLERR)) {
        m_logger.verbose("[" + m_addr_str +
                         "]: error: " + std::string(strerror(errno)));
        should_close = true;
    }

    if (!should_close && (events & EPOLLIN)) {
        std::vector<uint8_t> buf;
        bool again = true;

        while (again) {
            // process 100mb at a time
            Result<bool> res = recv(fd, buf, 104857600);
            if (!res.is_ok()) {
                m_logger.debug("[" + m_addr_str +
                               "]: recv failed: " + res.unwrap_err());

                should_close = true;
                break;
            }

            again = res.unwrap();

            if (m_message_cb) {
                m_message_cb(get_ptr(), buf);
            }
        }
    }

    if (!should_close && (events & EPOLLRDHUP)) {
        m_logger.verbose("[" + m_addr_str +
                         "]: connection closed by remote peer (RDHUP)");

        should_close = true;
    }

    if (should_close && m_disconnect_cb) {
        m_disconnect_cb(get_ptr());
        on_disconnect();
    }

    return !should_close;
}

} // namespace tcp