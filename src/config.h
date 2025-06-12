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

        LoggerType get_log_level() { return m_log_level; }

        std::string get_jdmb_host() { return m_jdmb_host; }
        int get_jdmb_port() { return m_jdmb_port; }
        int get_jdmb_queue_len() { return m_jdmb_queue_len; }

        std::string get_node_host() { return m_node_host; }
        int get_node_queue_len() { return m_node_queue_len; }

        std::string to_str();

        friend class Config;

      private:
        LoggerType m_log_level;

        std::string m_jdmb_host;
        int m_jdmb_port;
        int m_jdmb_queue_len;

        std::string m_node_host;
        int m_node_queue_len;
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