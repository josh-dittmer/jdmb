#include "jdmb.h"

#include "event/event_loop.h"
#include "util/str.h"

#include <thread>

bool JDMB::start() {
    m_logger.log("Starting JDMB v1.0 by Josh Dittmer");

    m_event_loop = EventLoop::create(m_logger_context);

    tcp::Client tcp_client = tcp::Client(m_logger_context, "localhost", 3000);

    tcp_client.on_connect([&](std::shared_ptr<tcp::ClientConnection> conn) {
        m_logger.log("Connection to discovery node [" + conn->get_addr_str() +
                     "] successful!");

        conn->on_message([&](std::shared_ptr<tcp::ClientConnection> conn,
                             const std::vector<uint8_t>& buf) -> bool {
            m_logger.log("Received data from [" + conn->get_addr_str() +
                         "]: " + util::str::bytes_to_str(buf));

            return true;
        });

        conn->send(util::str::str_to_bytes("Hello, world!"));
    });

    tcp_client.on_fail([&]() {
        m_logger.error("Connection to discovery node [" +
                       tcp_client.get_host() + ":" +
                       std::to_string(tcp_client.get_port()) + "] failed!");
        m_logger.error("Retrying in 5 seconds...");

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        discovery_node_connect(tcp_client, 5000);
    });

    discovery_node_connect(tcp_client, 5000);

    Result<> res = m_event_loop->start();
    if (!res.is_ok()) {
        m_logger.error("Event loop error: " + res.unwrap_err().get_msg());
        return false;
    }

    return true;
}

void JDMB::stop() {}

void JDMB::discovery_node_connect(tcp::Client tcp_client, int timeout) {
    Result<std::shared_ptr<tcp::ClientConnection>> tcp_conn =
        tcp_client.get_conn_hdl();

    if (!tcp_conn.is_ok()) {
        m_logger.error("TCP client error: " + tcp_conn.unwrap_err().get_msg());
    }

    m_event_loop->add(tcp_conn.unwrap(), timeout);
}
