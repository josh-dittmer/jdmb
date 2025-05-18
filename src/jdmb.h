#pragma once

#include "config.h"
#include "log/logger.h"
#include "log/logger_context.h"
#include "net/tcp/client.h"

#include <string>

class JDMB {
  public:
    JDMB(Config::Values config_values)
        : m_config_values(config_values),
          m_logger_context(std::make_shared<LoggerContext>(
              LoggerContext({m_config_values.get_log_level()}))),
          m_logger(m_logger_context->use("JDMB")) {}

    ~JDMB() {}

    bool start();
    void stop();

  private:
    void discovery_node_connect(tcp::Client tcp_client, int timeout);

    Config::Values m_config_values;

    std::shared_ptr<LoggerContext> m_logger_context;
    Logger m_logger;

    std::shared_ptr<EventLoop> m_event_loop;
};