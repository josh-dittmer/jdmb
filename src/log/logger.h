#pragma once

#include "logger_context.h"

#include <map>
#include <string>

class Logger {
  public:
    struct Format {
        std::string m_prefix;
        std::string m_color;
    };

    Logger(const LoggerContext::InstanceSettings& instance_settings)
        : m_instance_settings(instance_settings) {}
    ~Logger() {}

    void log(const std::string& msg) const;
    void fatal(const std::string& msg) const;
    void error(const std::string& msg) const;
    void warn(const std::string& msg) const;
    void debug(const std::string& msg) const;
    void verbose(const std::string& msg) const;

  private:
    static std::string _main_prefix;
    static std::map<LoggerType, Format> _format_table;

    void print(const std::string& prefix, const std::string& msg,
               const std::string& color, bool nl) const;

    std::string timestamp() const;

    LoggerContext::InstanceSettings m_instance_settings;
};