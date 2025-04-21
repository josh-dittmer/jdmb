#include "logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string Logger::_main_prefix = "[JDMB]";
std::map<LoggerType, Logger::Format> Logger::_format_table = {
    {LoggerType::LOG, {"[LOG]", "37"}},
    {LoggerType::FATAL, {"[FATAL]", "91"}},
    {LoggerType::ERROR, {"[ERROR]", "31"}},
    {LoggerType::WARN, {"[WARN]", "33"}},
    {LoggerType::DEBUG, {"[DEBUG]", "90"}},
    {LoggerType::VERBOSE, {"[VERBOSE]", "90"}}};

void Logger::log(const std::string& msg) const {
    static const Format& format = _format_table[LoggerType::LOG];
    if (m_instance_settings.m_settings->m_log_level >= LoggerType::LOG) {
        print(format.m_prefix, msg, format.m_color, true);
    }
}
void Logger::fatal(const std::string& msg) const {
    static const Format& format = _format_table[LoggerType::FATAL];
    if (m_instance_settings.m_settings->m_log_level >= LoggerType::FATAL) {
        print(format.m_prefix, msg, format.m_color, true);
    }
}
void Logger::error(const std::string& msg) const {
    static const Format& format = _format_table[LoggerType::ERROR];
    if (m_instance_settings.m_settings->m_log_level >= LoggerType::ERROR) {
        print(format.m_prefix, msg, format.m_color, true);
    }
}
void Logger::warn(const std::string& msg) const {
    static const Format& format = _format_table[LoggerType::WARN];
    if (m_instance_settings.m_settings->m_log_level >= LoggerType::WARN) {
        print(format.m_prefix, msg, format.m_color, true);
    }
}
void Logger::debug(const std::string& msg) const {
    static const Format& format = _format_table[LoggerType::DEBUG];
    if (m_instance_settings.m_settings->m_log_level >= LoggerType::DEBUG) {
        print(format.m_prefix, msg, format.m_color, true);
    }
}
void Logger::verbose(const std::string& msg) const {
    static const Format& format = _format_table[LoggerType::VERBOSE];
    if (m_instance_settings.m_settings->m_log_level >= LoggerType::VERBOSE) {
        print(format.m_prefix, msg, format.m_color, true);
    }
}

void Logger::print(const std::string& prefix, const std::string& msg,
                   const std::string& color, bool nl) const {
    const std::string spaces1 = std::string(10 - prefix.length(), ' ');
    const std::string spaces2 =
        std::string(15 - m_instance_settings.m_context.length(), ' ');

    std::ostringstream ss;

    ss << "\x1b[" << color << "m" << _main_prefix << " " << timestamp() << " "
       << prefix << spaces1 << "[" << m_instance_settings.m_context << "]"
       << spaces2 << msg << "\x1b[0m";

    if (nl)
        ss << "\n";

    std::cout << ss.str();
}

std::string Logger::timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);

    int ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 now.time_since_epoch())
                 .count() %
             1000;

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%m-%d-%y %H:%M:%S");
    ss << "." << std::setw(3) << std::setfill('0') << ms;

    return ss.str();
}