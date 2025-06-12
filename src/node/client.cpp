#pragma once

#include "../../net/tcp/client.h"

#include <string>

class Client {
  public:
    Client(const std::string& host, int port) : m_host(host), m_port(port) {}
    ~Client() {}

    void start(const std::shared_ptr<EventLoop>& event_loop);

  private:
    void on_connect(std::shared_ptr<tcp::Connection> connection);
    void on_fail();
    void on_message(std::shared_ptr<tcp::Connection> connection);
    void on_disconnect(std::shared_ptr<tcp::Connection> connection);

    std::string m_host;
    int m_port;

    std::vector<uint8_t> m_packet;
};