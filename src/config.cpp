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
            Error("failed to open file \"" + m_path + "\""));
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    rapidjson::Document doc;
    rapidjson::ParseResult res = doc.Parse(ss.str().c_str());

    if (!res) {
        std::string err_str =
            std::string(rapidjson::GetParseError_En(res.Code()));
        return Result<Values>::Err(err_str);
    }

    Result<int> port_res = read_int(doc, "port");
    if (!port_res.is_ok()) {
        return Result<Values>::Err(port_res.unwrap_err());
    }

    Result<bool> discovery_node_res = read_bool(doc, "discovery_node");
    if (!discovery_node_res.is_ok()) {
        return Result<Values>::Err(discovery_node_res.unwrap_err());
    }

    Result<std::string> log_level_res = read_str(doc, "log_level");
    if (!log_level_res.is_ok()) {
        return Result<Values>::Err(log_level_res.unwrap_err());
    }

    static std::map<std::string, LoggerType> str_to_log_level_map = {
        {"LOG", LoggerType::LOG},     {"FATAL", LoggerType::FATAL},
        {"ERROR", LoggerType::ERROR}, {"WARN", LoggerType::WARN},
        {"DEBUG", LoggerType::DEBUG}, {"VERBOSE", LoggerType::VERBOSE},
    };

    auto mit = str_to_log_level_map.find(log_level_res.unwrap());
    if (mit == str_to_log_level_map.end()) {
        return Result<Values>::Err(Error("invalid log level"));
    }

    LoggerType log_level = mit->second;

    Values values;
    values.m_port = port_res.unwrap();
    values.m_discovery_node = discovery_node_res.unwrap();
    values.m_log_level = log_level;

    return Result<Values>::Ok(values);
}

Result<std::string> Config::read_str(const rapidjson::Document& doc,
                                     const std::string& key) {
    if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsString()) {
        return Result<std::string>::Err(
            Error("failed to read required string \"" + key + "\""));
    }

    return Result<std::string>::Ok(std::string(doc[key.c_str()].GetString()));
}
Result<int> Config::read_int(const rapidjson::Document& doc,
                             const std::string& key) {
    if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsInt()) {
        return Result<int>::Err(
            Error("failed to read required integer \"" + key + "\""));
    }

    return Result<int>::Ok(doc[key.c_str()].GetInt());
}

Result<bool> Config::read_bool(const rapidjson::Document& doc,
                               const std::string& key) {
    if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsBool()) {
        return Result<bool>::Err(
            Error("failed to read required boolean \"" + key + "\""));
    }

    return Result<bool>::Ok(doc[key.c_str()].GetBool());
}