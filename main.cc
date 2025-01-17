#include "app/include/runner.h"
#include "common/const.h"
#include "config/trace_agent_config.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <cstring>
#include <fmt/core.h>
#include <memory>

// Define the flags, keep
ABSL_FLAG(std::string, c, "", "Path to the configuration file");
ABSL_FLAG(bool, v, false, "Show version");
#define VERSION "0.1"

#ifndef GIT_HASH
#define GIT_HASH "unknown"
#endif

#ifdef USE_DEBUG
static constexpr std::string_view kBuildType = "debug version";
#else
static constexpr std::string_view kBuildType = "release version";
#endif

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    if (absl::GetFlag(FLAGS_v)) {
        fmt::println("version: {}, build: {}, {}", VERSION, GIT_HASH, kBuildType);
        return 0;
    }

    std::string const config_file = absl::GetFlag(FLAGS_c);
    if (config_file.empty()) {
        fmt::println("The configuration file cannot be empty");
        return -1;
    }

    // read the config file
    auto config = std::make_shared<App::Config::ConfigReader>();
    if (!config->ReadConfigFromFile(config_file)) {
        fmt::println("parse config file failed.");
        return -1;
    }

    if (config->GetConfig().close_stdout()) {
        // 重定向 stdout到 /tmp/stdout.log
        if (freopen(App::Common::stdout_path.c_str(), "ae", stdout) == nullptr) {
            fmt::println("Failed to redirect stdout, errno={}, message={}", errno, strerror(errno));
            exit(-1);
        }

        // 重定向 stderr到 /tmp/stderr.log
        if (freopen(App::Common::stderr_path.c_str(), "ae", stderr) == nullptr) {
            fmt::println("Failed to redirect stderr, errno={}, message={}", errno, strerror(errno));
            exit(-1);
        }
    }

    auto loglevel = config->LogLevel();
    spdlog::level::level_enum log_level_enum = spdlog::level::from_str(loglevel);
    spdlog::set_level(log_level_enum);

    // set log path
    auto log_file = config->LogFile();
    if (!log_file.empty()) {
        auto logger = spdlog::basic_logger_mt("trace-agent", log_file);
        spdlog::set_default_logger(logger);
    }

    auto runner = new App::Runner(config);
    runner->run();
    delete runner;
    return 0;
}
