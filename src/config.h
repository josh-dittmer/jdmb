#pragma once

#include "util/result.h"

#include "log/logger_type.h"

#include <rapidjson/document.h>

class Config {
  public:
    class Values {
      public:
        Values() {}
        ~Values() {}

        int get_port() const { return m_port; }
        bool is_discovery_node() { return m_discovery_node; }
        LoggerType get_log_level() { return m_log_level; }

        std::string to_str();

        friend class Config;

      private:
        int m_port;
        bool m_discovery_node;
        LoggerType m_log_level;
    };

    Config(const std::string& path) : m_path(path) {}
    ~Config() {}

    Result<Values> load();

  private:
    Result<std::string> read_str(const rapidjson::Document& doc,
                                 const std::string& key);
    Result<int> read_int(const rapidjson::Document& doc,
                         const std::string& key);
    Result<bool> read_bool(const rapidjson::Document& doc,
                           const std::string& key);

    std::string m_path;
};