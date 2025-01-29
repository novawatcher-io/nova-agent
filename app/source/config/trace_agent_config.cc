#include "config/nova_agent_config.h"
#include "common/file.h"
#include <google/protobuf/json/json.h>
#include <google/protobuf/util/json_util.h>
using google::protobuf::util::JsonStringToMessage;

namespace App::Config {
bool ConfigReader::ReadConfigFromFile(const std::string& path) {
    App::Common::BasicFileReader reader;
    std::string content;
    if (!reader.ReadFile(path, &content)) {
        SPDLOG_ERROR("fail to read config file: {}", path);
        return false;
    }
    // convert string to pb
    auto ret = JsonStringToMessage(content, &config_);
    if (!ret.ok()) {
        SPDLOG_ERROR("parse failed: {}", ret.message());
        return false;
    }
    SPDLOG_DEBUG("config: {}", config_.ShortDebugString());

    return true;
}

std::string ConfigReader::NodeReportHost() const {
    return fmt::format("{}:{}", config_.node_report_addr().host(), config_.node_report_addr().port());
}
std::string ConfigReader::OLTPExporterAddress() const {
    return fmt::format("{}:{}", config_.oltp_exporter_addr().host(), config_.oltp_exporter_addr().port());
}
} // namespace App::Config
