#include "server.h"

#include "../util/debug.h"
#include "../util/str.h"

namespace node {

void Server::Connection::on_connect(std::shared_ptr<tcp::Connection> tcp_conn) {
    m_logger.log("[" + tcp_conn->get_addr_str() + "]: connected");
    m_bytes_needed = 4;
}

bool Server::Connection::on_message(std::shared_ptr<tcp::Connection> tcp_conn,
                                    const std::vector<uint8_t>& data) {
    while (true) {
        if (m_bytes_needed <= 4) {
        }
    }
    if (m_curr_packet.empty()) {
        if (data.size() < 4) {
            m_logger.debug("[" + tcp_conn->get_addr_str() +
                           "]: invalid length header");
            return false;
        }

        m_expected_packet_len =
            (data[0]) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

        m_curr_packet.insert(m_curr_packet.end(), data.begin() + 4, data.end());
    } else {
        m_curr_packet.insert(m_curr_packet.end(), data.begin(), data.end());
    }

    if (data.size() >= m_expected_packet_len) {
    }

    m_logger.log("[" + tcp_conn->get_addr_str() + "]: total " +
                 std::to_string(m_curr_packet.size()) + " bytes (" +
                 std::to_string(m_expected_packet_len) +
                 " needed for current packet)");
    return true;
}

void Server::Connection::on_disconnect(
    std::shared_ptr<tcp::Connection> tcp_conn) {
    m_logger.log("[" + tcp_conn->get_addr_str() + "]: disconnected");
}

Server::Server(Private, std::shared_ptr<LoggerContext> logger_context,
               const std::string& host, int port, int max_queue_len)
    : m_logger_context(logger_context),
      m_logger(logger_context->use("NodeServer")),
      m_tcp_server(
          tcp::Server::create(logger_context, host, port, max_queue_len)) {
    m_tcp_server->on_connect(
        std::bind(&Server::on_connect, this, std::placeholders::_1));
}

Result<std::shared_ptr<Server>>
Server::create(std::shared_ptr<EventLoop> event_loop,
               std::shared_ptr<LoggerContext> logger_context,
               const std::string& host, int port, int max_queue_len) {
    std::shared_ptr server = std::make_shared<Server>(
        Private(), logger_context, host, port, max_queue_len);

    Result<> ev_res = event_loop->add(server->m_tcp_server);
    if (!ev_res.is_ok()) {
        return Result<std::shared_ptr<Server>>::Err(Error(__func__, ev_res));
    }

    return Result<std::shared_ptr<Server>>::Ok(server);
}

void Server::on_connect(std::shared_ptr<tcp::Connection> tcp_conn) {
    std::shared_ptr<Connection> conn = Connection::create(m_logger_context);

    tcp_conn->on_message(std::bind(&Connection::on_message, conn.get(),
                                   std::placeholders::_1,
                                   std::placeholders::_2));
    tcp_conn->on_disconnect(std::bind(&Connection::on_disconnect, conn.get(),
                                      std::placeholders::_1));

    conn->on_connect(tcp_conn);
    m_connections.insert(std::make_pair(tcp_conn, conn));

    m_logger.debug("[" + tcp_conn->get_addr_str() + "]: connection added");
}

void Server::on_disconnect(std::shared_ptr<tcp::Connection> tcp_conn) {
    std::size_t num = m_connections.erase(tcp_conn);
    m_logger.debug("[" + tcp_conn->get_addr_str() +
                   "]: " + std::to_string(num) + " connection(s) removed");
}

} // namespace node