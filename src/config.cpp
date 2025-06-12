#include "config.h"

#include <rapidjson/error/en.h>

#include <fstream>
#include <map>
#include <sstream>

std::string Config::Values::to_str() { return ""; }

Result<Config::Values> Config::load() {
    std::ifstream file(m_path);
    if (!file.good()) {
        return Result<Values>::Err(
            Error(__func__, "failed to open file \"" + m_path + "\""));
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    rapidjson::Document doc;
    rapidjson::ParseResult res = doc.Parse(ss.str().c_str());

    if (!res) {
        std::string err_str =
            std::string(rapidjson::GetParseError_En(res.Code()));
        return Result<Values>::Err(Error(__func__, err_str));
    }

    Result<std::string> log_level_res = read_str(doc, "log_level");
    if (!log_level_res.is_ok()) {
        return Result<Values>::Err(Error(__func__, log_level_res));
    }

    static std::map<std::string, LoggerType> str_to_log_level_map = {
        {"LOG", LoggerType::LOG},     {"FATAL", LoggerType::FATAL},
        {"ERROR", LoggerType::ERROR}, {"WARN", LoggerType::WARN},
        {"DEBUG", LoggerType::DEBUG}, {"VERBOSE", LoggerType::VERBOSE},
    };

    auto mit = str_to_log_level_map.find(log_level_res.unwrap());
    if (mit == str_to_log_level_map.end()) {
        return Result<Values>::Err(Error(__func__, "invalid log level"));
    }

    LoggerType log_level = mit->second;

    Result<std::string> jdmb_host_res = read_str(doc, "jdmb_host");
    if (!jdmb_host_res.is_ok()) {
        return Result<Values>::Err(Error(__func__, jdmb_host_res));
    }

    Result<int> jdmb_port_res = read_int(doc, "jdmb_port");
    if (!jdmb_port_res.is_ok()) {
        return Result<Values>::Err(Error(__func__, jdmb_port_res));
    }

    Result<int> jdmb_queue_len_res = read_int(doc, "jdmb_queue_len");
    if (!jdmb_queue_len_res.is_ok()) {
        return Result<Values>::Err(Error(__func__, jdmb_queue_len_res));
    }

    Result<std::string> node_host_res = read_str(doc, "node_host");
    if (!node_host_res.is_ok()) {
        return Result<Values>::Err(Error(__func__, node_host_res));
    }

    Result<int> node_queue_len_res = read_int(doc, "node_queue_len");
    if (!node_queue_len_res.is_ok()) {
        return Result<Values>::Err(Error(__func__, node_queue_len_res));
    }

    Values values;
    values.m_log_level = log_level;
    values.m_jdmb_host = jdmb_host_res.unwrap();
    values.m_jdmb_port = jdmb_port_res.unwrap();
    values.m_jdmb_queue_len = jdmb_queue_len_res.unwrap();
    values.m_node_host = node_host_res.unwrap();
    values.m_node_queue_len = node_queue_len_res.unwrap();

    return Result<Values>::Ok(values);
}

Result<std::string> Config::read_str(const rapidjson::Document& doc,
                                     const std::string& key) {
    if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsString()) {
        return Result<std::string>::Err(
            Error(__func__, "failed to read required string \"" + key + "\""));
    }

    return Result<std::string>::Ok(std::string(doc[key.c_str()].GetString()));
}
Result<int> Config::read_int(const rapidjson::Document& doc,
                             const std::string& key) {
    if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsInt()) {
        return Result<int>::Err(
            Error(__func__, "failed to read required integer \"" + key + "\""));
    }

    return Result<int>::Ok(doc[key.c_str()].GetInt());
}

Result<bool> Config::read_bool(const rapidjson::Document& doc,
                               const std::string& key) {
    if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsBool()) {
        return Result<bool>::Err(
            Error(__func__, "failed to read required boolean \"" + key + "\""));
    }

    return Result<bool>::Ok(doc[key.c_str()].GetBool());
}