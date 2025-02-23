//
// Created by root on 2024/4/20.
//
#pragma once

#include <component/api.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <opentelemetry-proto/opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.h>

#include "app/include/common/opentelemetry/otlp_grpc_client.h"
#include "app/include/common/opentelemetry/otlp_grpc_client_options.h"

namespace App {
namespace Sink {
namespace OpenTelemetry {
namespace Log {
namespace Grpc {
using namespace opentelemetry::proto::collector::logs::v1;
using namespace App::Common::Opentelemetry;
class Sink : public Core::Component::Consumer {
public:
    explicit Sink(std::unique_ptr<grpc::CompletionQueue>& cq) {
        auto options = std::make_shared<OtlpGrpcClientOptions>();
        options->max_concurrent_requests = 1000;
        const char* host = getenv("SINK_HOST");
        if (host == nullptr) {
            host = "localhost:4317";
        }
        options->endpoint = host;
        client = std::make_unique<OtlpGrpcClient>(options, cq.get());
        logs_service_stub_ = OtlpGrpcClient::MakeLogsServiceStub(*options);
    }

    std::shared_ptr<Core::Component::Queue>& channel() final {
        return queue;
    }

    Core::Component::Result Consume(std::shared_ptr<Core::Component::Batch>&) override {
        return {};
    }

    Core::Component::Result Consume(Core::Component::Batch& ) {
        return {};
    }

    bool bindChannel(std::shared_ptr<Core::Component::Queue>) final {
        return true;
    }

    void stop() final {
        client->Shutdown();
    }

private:
    std::shared_ptr<Core::Component::Queue> queue;
    std::unique_ptr<LogsService::Stub> logs_service_stub_;
    std::unique_ptr<OtlpGrpcClient> client;
};
} // namespace Grpc
} // namespace Log
} // namespace OpenTelemetry
} // namespace Sink
} // namespace App
