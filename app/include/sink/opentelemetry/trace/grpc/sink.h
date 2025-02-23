/**
******************************************************************************
* @file           : sink.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/3/2
******************************************************************************
*/
//
// Created by zhanglei on 2024/3/2.
//

#pragma once

#include "app/include/common/opentelemetry/export_trace_service_call_data.h"
#include "app/include/common/opentelemetry/otlp_grpc_client.h"
#include "app/include/common/opentelemetry/otlp_grpc_client_options.h"
#include "opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h"
#include <component/api.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <opentelemetry-proto/opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>

namespace App {
namespace Sink {
namespace OpenTelemetry {
namespace Trace {
namespace Grpc {
using namespace App::Common::Opentelemetry;
using namespace opentelemetry::proto::collector::trace::v1;
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
        trace_service_stub_ = OtlpGrpcClient::MakeTraceServiceStub(*options);
    }

    virtual std::shared_ptr<Core::Component::Queue>& channel() {
        return queue;
    }

    virtual bool bindChannel(std::shared_ptr<Core::Component::Queue>) {
        return true;
    }

    Core::Component::Result Consume(std::shared_ptr<Core::Component::Batch>&) override {
        return {};
    }

    void stop() final {
        client->Shutdown();
    }

    Core::Component::Result Consume(Core::Component::Batch& batch);

private:
    std::shared_ptr<Core::Component::Queue> queue;
    std::unique_ptr<TraceService::Stub> trace_service_stub_;
    std::unique_ptr<grpc::CompletionQueue>& cq_;
    std::unique_ptr<OtlpGrpcClient> client;
};
} // namespace Grpc
} // namespace Trace
} // namespace OpenTelemetry
} // namespace Sink
} // namespace App