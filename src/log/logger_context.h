#pragma once

#include "logger_type.h"

#include <iostream>
#include <memory>

class LoggerContext {
  public:
    struct Settings {
        LoggerType m_log_level;
    };

    struct InstanceSettings {
        std::string m_context;
        std::shared_ptr<Settings> m_settings;
    };

    LoggerContext(Settings settings)
        : m_settings(std::make_shared<Settings>(settings)) {}
    ~LoggerContext() {}

    InstanceSettings use(const std::string& context);

    void set_log_level(const LoggerType& log_level) {
        m_settings->m_log_level = log_level;
    }

  private:
    std::shared_ptr<Settings> m_settings;
};