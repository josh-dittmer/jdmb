#include "jdmb.h"

#include "event/fd/fd.h"
#include "util/str.h"

#include <functional>

struct CommandLineArgs {
    std::string m_config_path;

    bool m_init_cluster;
    std::string m_discover_host;
};

CommandLineArgs read_args(const Logger& main_logger, int argc, char* argv[]) {
    CommandLineArgs args;

    // default args
    args.m_init_cluster = false;
    args.m_discover_host = "";
    args.m_config_path = "conf/conf.json";

    static std::map<std::string, std::function<void(const std::string&)>>
        parse_map = {
            {"--init-cluster",
             [&](const std::string& val) {
                 args.m_init_cluster =
                     util::str::to_lower(val) == "false" ? false : true;
             }},
            {"--discover-host",
             [&](const std::string& val) { args.m_discover_host = val; }},
            {"--conf-path",
             [&](const std::string& val) { args.m_config_path = val; }}};

    for (int i = 1; i < argc; i++) {
        std::string arg_str(argv[i]);

        std::vector<std::string> arg_str_split = util::str::split(arg_str, '=');
        std::string arg = arg_str_split[0];
        std::string val = arg_str_split.size() > 1 ? arg_str_split[1] : "";

        auto mit = parse_map.find(arg);
        if (mit == parse_map.end()) {
            main_logger.warn("unknown argument \"" + arg + "\"");
            continue;
        }

        mit->second(val);
    }

    std::string args_str = "\n\tinit-cluster=" +
                           std::string(args.m_init_cluster ? "true" : "false") +
                           "\n\tdiscover-host=" + args.m_discover_host +
                           "\n\tconf-path=" + args.m_config_path;

    main_logger.log("Starting with arguments: " + args_str);

    return args;
}

int start(int argc, char* argv[]) {
    // main logger, log level will always be ERROR regardless of config
    LoggerContext::Settings lc_settings;
    lc_settings.m_log_level = LoggerType::DEBUG;

    LoggerContext main_logger_context = LoggerContext(lc_settings);
    Logger main_logger = Logger(main_logger_context.use("Main"));

    // read args
    CommandLineArgs args = read_args(main_logger, argc, argv);

    if (!args.m_init_cluster && args.m_discover_host.empty()) {
        main_logger.fatal(
            "Argument \"discover-host\" is required if cluster already exists");
        main_logger.fatal("JDMB exited with non-zero status code");
        return -1;
    }

    // load config
    Config config = Config(args.m_config_path);
    Result<Config::Values> config_res = config.load();

    if (!config_res.is_ok()) {
        main_logger.fatal("Failed to load config: " + config_res.unwrap_err());
        main_logger.fatal("JDMB exited with non-zero status code");
        return -1;
    }

    Config::Values config_values = config_res.unwrap();

    JDMB jdmb = JDMB(config_values);
    if (!jdmb.start(args.m_init_cluster, args.m_discover_host)) {
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