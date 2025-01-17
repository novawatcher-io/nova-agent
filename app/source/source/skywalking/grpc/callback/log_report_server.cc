#include "app/include/source/skywalking/grpc/callback/log_report_server.h"
#include "app/include/source/skywalking/grpc/logging_server_event_data.h"
#include <spdlog/spdlog.h>

namespace App::Source::SkyWalking::Grpc::Callback {
using namespace ::skywalking::v3;
::grpc::Status LogReportServer::collect(::grpc::ServerContext* context,
                                        ::grpc::ServerReader<::skywalking::v3::LogData>* reader,
                                        ::skywalking::v3::Commands* response) {
    const auto& metadata = context->client_metadata();
    grpc::string_ref authentication;
    auto it = metadata.find("authentication");
    if (it != metadata.end() && !it->second.empty()) {
        // SPDLOG_INFO("trace server token: {}", it->second.data());
        authentication = it->second;
    } else {
        SPDLOG_WARN("trace server token is empty");
        return ::grpc::Status::OK;
    }

    while (reader->Read(&request)) {
        if (!request.IsInitialized()) {
            continue;
        }
        std::unique_ptr<::skywalking::v3::LogData> collection = std::make_unique<::skywalking::v3::LogData>();
        collection->CopyFrom(request);
        auto segmentData = std::make_unique<Core::Component::AnyData<LogData>>(std::move(collection));
        auto data = std::make_unique<LoggingServerEventData>(
            std::make_unique<Core::Component::AnyData<::skywalking::v3::LogData>>(std::move(collection)));
        if (!authentication.empty()) {
            data->addMeta("authentication", std::string(authentication.data(), authentication.size()));
        }
        logPipeline->push(std::move(data));
    }
    return ::grpc::Status::OK;
}
} // namespace App::Source::SkyWalking::Grpc::Callback
