#pragma once

#include <bits/chrono.h>
#include <grpcpp/grpcpp.h>
#include <opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.h>
#include <opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h>
#include <opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>
#include <atomic>
#include <memory>
#include "export_trace_service_call_data.h"
#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h"
#include "opentelemetry/proto/collector/trace/v1/trace_service.pb.h"
#include "opentelemetry/sdk/common/exporter_utils.h"
namespace App { namespace Common { namespace Opentelemetry { struct OtlpGrpcClientAsyncData; } } }  // lines 24-24
namespace App { namespace Common { namespace Opentelemetry { struct OtlpGrpcClientOptions; } } }  // lines 25-25
namespace google { namespace protobuf { class Arena; } }

using namespace opentelemetry;
namespace App {
namespace Common {
namespace Opentelemetry {
using namespace opentelemetry::proto::collector::metrics::v1;
class OtlpGrpcClient {
public:
    OtlpGrpcClient(std::shared_ptr<OtlpGrpcClientOptions>& options, grpc::CompletionQueue* cq);

    ~OtlpGrpcClient();

    /**
     * Create gRPC channel from the exporter options.
     */
    static std::shared_ptr<grpc::Channel> MakeChannel(const OtlpGrpcClientOptions& options);

    /**
     * Create gRPC client context to call RPC.
     */
    static std::unique_ptr<grpc::ClientContext> MakeClientContext(const OtlpGrpcClientOptions& options);

    /**
     * Create trace service stub to communicate with the OpenTelemetry Collector.
     */
    static std::unique_ptr<proto::collector::trace::v1::TraceService::Stub>
    MakeTraceServiceStub(const OtlpGrpcClientOptions& options);

    /**
     * Create trace service stub to communicate with the OpenTelemetry Collector.
     */
    static std::unique_ptr<proto::collector::metrics::v1::MetricsService::Stub>
    MakeMetricsServiceStub(const OtlpGrpcClientOptions& options);

    static std::unique_ptr<proto::collector::logs::v1::LogsService::Stub>
    MakeLogsServiceStub(const OtlpGrpcClientOptions& options);

    sdk::common::ExportResult Export(
    std::unique_ptr<TraceService::Stub>& stub, ExportTraceServiceRequest&& request,
        std::unique_ptr<google::protobuf::Arena>&& arena,
        std::function<void(const grpc::Status&, const ExportTraceServiceResponse&)> asyncCallable);

    sdk::common::ExportResult Export(
    std::unique_ptr<MetricsService::Stub>& stub, proto::collector::metrics::v1::ExportMetricsServiceRequest&& request,
        std::unique_ptr<google::protobuf::Arena>&& arena,
        std::function<void(const grpc::Status&, const proto::collector::metrics::v1::ExportMetricsServiceResponse&)> asyncCallable) {
        return opentelemetry::sdk::common::ExportResult::kSuccess;
    }

    /**
     * Force flush the gRPC client.
     */
    bool ForceFlush(std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept;

    /**
     * Shut down the gRPC client.
     * @param timeout an optional timeout, the default timeout of 0 means that no
     * timeout is applied.
     * @return return the status of this operation
     */
    bool Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept;

    std::shared_ptr<OtlpGrpcClientAsyncData> MutableAsyncData(const OtlpGrpcClientOptions& options);

    std::shared_ptr<OtlpGrpcClientAsyncData> checkRunStatus();

private:
    // Stores if this gRPC client had its Shutdown() method called
    std::atomic<bool> is_shutdown_;

    // Stores shared data between threads of this gRPC client
    std::shared_ptr<OtlpGrpcClientAsyncData> async_data_;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    grpc::CompletionQueue* cq_;

    std::shared_ptr<OtlpGrpcClientOptions> options;
};

}
}
}
