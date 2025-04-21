#include "logger_context.h"

LoggerContext::InstanceSettings LoggerContext::use(const std::string& context) {
    InstanceSettings instance_settings;
    instance_settings.m_context =
        (context.length() > 11) ? context.substr(0, 11) : context;
    instance_settings.m_settings = m_settings;

    return instance_settings;
}