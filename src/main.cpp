#include <iostream>

#include "jdmb.h"

#include "event/fd/fd.h"

int start(int argc, char* argv[]) {
    // main logger, log level will always be ERROR regardless of config
    LoggerContext::Settings lc_settings;
    lc_settings.m_log_level = LoggerType::DEBUG;

    LoggerContext main_logger_context = LoggerContext(lc_settings);
    Logger main_logger = Logger(main_logger_context.use("Main"));

    // load config
    std::string config_path =
        argc < 2 ? "conf/conf.json" : std::string(argv[1]);

    Config config = Config(config_path);
    Result<Config::Values> config_res = config.load();

    if (!config_res.is_ok()) {
        main_logger.fatal("Failed to load config: " + config_res.unwrap_err());
        main_logger.fatal("JDMB exited with non-zero status code");
        return -1;
    }

    Config::Values config_values = config_res.unwrap();

    JDMB jdmb = JDMB(config_values);
    if (!jdmb.start()) {
        main_logger.fatal("JDMB exited with non-zero status code");
        return -1;
    }

    main_logger.log("JDMB stopped, exiting gracefully");

    return 0;
}

int main(int argc, char* argv[]) {
    int res = start(argc, argv);

    std::cout << FD::_OPEN_COUNT << " fds opened" << std::endl;
    std::cout << FD::_CLOSE_COUNT << " fds closed" << std::endl;

    return res;
}