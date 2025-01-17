#include "app/include/source/skywalking/grpc/callback/jvm_metric_report_server.h"
#include "app/include/source/skywalking/grpc/jvm_metric_server_event_data.h"
#include "common/Command.pb.h"
#include "language-agent/JVMMetric.pb.h"
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <memory>
#include <spdlog/spdlog.h>

namespace App::Source::SkyWalking::Grpc::Callback {
::grpc::Status JvmMetricReportServer::collect(::grpc::ServerContext* context,
                                              const ::skywalking::v3::JVMMetricCollection* request,
                                              ::skywalking::v3::Commands* /*response*/) {
    if (request->IsInitialized()) {
        return ::grpc::Status::OK;
    }
    grpc::string_ref authentication;
    const auto& metadata = context->client_metadata();
    auto it = metadata.find("authentication");
    if (it != metadata.end() && !it->second.empty()) {
        SPDLOG_INFO("trace server token: {}", it->second.data());
        authentication = it->second;
    }

    auto collection = std::make_unique<::skywalking::v3::JVMMetricCollection>();
    collection->CopyFrom(*request);
    auto data = std::make_unique<JvmMetricServerEventData>(
        std::make_unique<Core::Component::AnyData<::skywalking::v3::JVMMetricCollection>>(std::move(collection)));
    if (!authentication.empty()) {
        data->addMeta("authentication", std::string(authentication.data(), authentication.size()));
    }
    metricPipeline->push(std::move(data));
    return ::grpc::Status::OK;
}
} // namespace App::Source::SkyWalking::Grpc::Callback
