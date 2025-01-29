#pragma once
#include "proto/nova_agent_config.pb.h"
#include "app/include/common/const.h"
#include <memory>
#include <string>

namespace App::Config {

class ConfigReader {
public:
    bool ReadConfigFromFile(const std::string& path);
    const nova_agent::config::TraceAgentConfig& GetConfig() const {
        return config_;
    }
    std::string NodeReportHost() const;
    std::string OLTPExporterAddress() const;
    std::string LogLevel() const {
        auto ret = config_.log_level();
        if (ret.empty()) {
            return App::Common::Log::info;
        }
        return ret;
    }
    std::string LogFile() const {
        auto path = config_.log_file();
        if (path.empty() || path == App::Common::stdout) {
            return "";
        }
        return path;
    }

private:
    nova_agent::config::TraceAgentConfig config_;
};
} // namespace App::Config
