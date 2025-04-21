#pragma once

#include "config.h"
#include "log/logger.h"
#include "log/logger_context.h"

#include <string>

class JDMB {
  public:
    JDMB(Config::Values config_values)
        : m_config_values(config_values),
          m_logger_context({m_config_values.get_log_level()}),
          m_logger(m_logger_context.use("JDMB")) {}

    ~JDMB() {}

    bool start();
    void stop();

  private:
    Config::Values m_config_values;

    LoggerContext m_logger_context;
    Logger m_logger;
};