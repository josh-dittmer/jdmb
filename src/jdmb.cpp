#include "jdmb.h"

#include "event/event_loop.h"
#include "util/str.h"

#include <thread>

bool JDMB::start() {
    m_logger.log("Starting JDMB v1.0 by Josh Dittmer");

    m_event_loop = EventLoop::create(m_logger_context);

    /*tcp::Client tcp_client1 = tcp::Client(m_logger_context, "localhost",
    3000); configure_test_client(tcp_client1);

    discovery_node_connect(tcp_client1, 5000);

    tcp::Client tcp_client2 = tcp::Client(m_logger_context, "localhost", 3000);
    configure_test_client(tcp_client2);

    discovery_node_connect(tcp_client2, 5000);*/

    std::shared_ptr<tcp::Server> tcp_server =
        tcp::Server::create(m_logger_context, 4444, 128);
    configure_test_server(*tcp_server);

    m_event_loop->add(tcp_server);

    Result<> res = m_event_loop->start();
    if (!res.is_ok()) {
        m_logger.error("Event loop error: " + res.unwrap_err());
        return false;
    }

    return true;
}

void JDMB::stop() {}

void JDMB::configure_test_client(tcp::Client& tcp_client) {
    tcp_client.on_connect([&](std::shared_ptr<tcp::Connection> conn) {
        m_logger.log("Connection to discovery node [" + conn->get_addr_str() +
                     "] successful!");

        conn->on_message([&](std::shared_ptr<tcp::Connection> conn,
                             const std::vector<uint8_t>& buf) -> bool {
            m_logger.log("Received data from [" + conn->get_addr_str() +
                         "]: " + util::str::bytes_to_str(buf));

            return true;
        });

        conn->on_disconnect([&](std::shared_ptr<tcp::Connection> conn) {
            m_logger.log("Connection to discovery node [" +
                         conn->get_addr_str() + "] closed!");

            discovery_node_connect(tcp_client, 5000);
        });

        //
        const int test_buf_size = 65536;

        std::vector<uint8_t> test_buf;
        test_buf.reserve(test_buf_size);

        for (int i = 0; i < test_buf_size; i++) {
            test_buf.push_back((uint8_t)'A');
        }

        for (int i = 0; i < 1000; i++) {
            conn->send(test_buf);
        }
        //
    });

    tcp_client.on_fail([&]() {
        m_logger.error("Connection to discovery node [" +
                       tcp_client.get_host() + ":" +
                       std::to_string(tcp_client.get_port()) + "] failed!");
        /*m_logger.error("Retrying in 5 seconds...");

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        discovery_node_connect(tcp_client, 5000);*/
    });
}

void JDMB::discovery_node_connect(const tcp::Client& tcp_client, int timeout) {
    Result<std::shared_ptr<tcp::Client::Connection>> tcp_conn =
        tcp_client.get_conn_hdl();

    if (!tcp_conn.is_ok()) {
        m_logger.error("TCP client error: " + tcp_conn.unwrap_err());
    }

    m_event_loop->add(tcp_conn.unwrap(), timeout);
}

void JDMB::configure_test_server(tcp::Server& tcp_server) {
    tcp_server.on_connect([&](std::shared_ptr<tcp::Connection> conn) {
        m_logger.log("[" + conn->get_addr_str() + "] connected");

        conn->on_message([&](std::shared_ptr<tcp::Connection> conn,
                             const std::vector<uint8_t>& buf) -> bool {
            // m_logger.log("[" + conn->get_addr_str() + "]: received \"" +
            //              util::str::bytes_to_str(buf) + "\"");

            m_logger.log("[" + conn->get_addr_str() + "]: received " +
                         std::to_string(buf.size()) + " bytes");

            return true;
        });

        conn->on_disconnect([&](std::shared_ptr<tcp::Connection> conn) {
            m_logger.log("[" + conn->get_addr_str() + "] disconnected");
        });

        // conn->send(util::str::str_to_bytes("Hello, world!"));

        //
        const int test_buf_size = 65536;

        std::vector<uint8_t> test_buf;
        test_buf.reserve(test_buf_size);

        for (int i = 0; i < test_buf_size; i++) {
            test_buf.push_back((uint8_t)'A');
        }

        for (int i = 0; i < 100; i++) {
            conn->send(test_buf);
        }
        //
    });
}