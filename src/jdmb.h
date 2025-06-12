#pragma once

#include "config.h"
#include "event/event_loop.h"
#include "log/logger.h"
#include "log/logger_context.h"

#include <string>

class JDMB {
  public:
    JDMB(Config::Values config_values)
        : m_config_values(config_values),
          m_logger_context(std::make_shared<LoggerContext>(
              LoggerContext({m_config_values.get_log_level()}))),
          m_logger(m_logger_context->use("JDMB")) {}

    ~JDMB() {}

    bool start(bool init_cluster, const std::string& discover_host);
    void stop();

    static const int NodePort;

  private:
    // bool configure_node_server(const std::string& node_host);
    bool perform_discovery(const std::string& discover_host);

    /*void configure_test_client(tcp::Client& tcp_client);
    void discovery_node_connect(const tcp::Client& tcp_client, int timeout);

    void configure_test_server(tcp::Server& tcp_server);*/

    Config::Values m_config_values;

    std::shared_ptr<LoggerContext> m_logger_context;
    Logger m_logger;

    std::shared_ptr<EventLoop> m_event_loop;
};