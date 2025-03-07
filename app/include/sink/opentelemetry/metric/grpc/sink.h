//
// Created by root on 2024/4/12.
//
#pragma once

#include "app/include/common/opentelemetry/otlp_grpc_client.h"
#include "app/include/common/opentelemetry/otlp_grpc_client_options.h"
#include <component/api.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <opentelemetry-proto/opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h>

namespace App {
namespace Sink {
namespace OpenTelemetry {
namespace Metric {
namespace Grpc {
using namespace opentelemetry::proto::collector::metrics::v1;
using namespace App::Common::Opentelemetry;
class Sink : public Core::Component::Consumer {
public:
    Sink(std::unique_ptr<grpc::CompletionQueue>& cq) : cq_(cq) {
        auto options = std::make_shared<OtlpGrpcClientOptions>();
        options->max_concurrent_requests = 1000;
        const char* host = getenv("SINK_HOST");
        if (host == nullptr) {
            host = "localhost:4317";
        }
        options->endpoint = host;
        client = std::make_unique<OtlpGrpcClient>(options, cq.get());
        metric_service_stub_ = OtlpGrpcClient::MakeMetricsServiceStub(*options);
    }


    std::shared_ptr<Core::Component::Queue>& channel() final {
        return queue;
    }

    bool bindChannel(std::shared_ptr<Core::Component::Queue> ) final {
        return true;
    }

    Core::Component::Result Consume(std::shared_ptr<Core::Component::Batch>& batch) {
        return Consume(*batch);
    }

    void stop() final {
        is_shutdown_ = true;
        client->Shutdown();
    }

    Core::Component::Result Consume(Core::Component::Batch& batch);

private:
    std::shared_ptr<Core::Component::Queue> queue;
    std::unique_ptr<MetricsService::Stub> metric_service_stub_;
    std::unique_ptr<grpc::CompletionQueue>& cq_;
    std::unique_ptr<OtlpGrpcClient> client;
    // Stores if this gRPC client had its Shutdown() method called
    std::atomic<bool> is_shutdown_;
};
} // namespace Grpc
} // namespace Metric
} // namespace OpenTelemetry
} // namespace Sink
} // namespace App