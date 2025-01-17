#include <app/include/common/opentelemetry/otlp_grpc_client.h>
#include <app/include/common/opentelemetry/otlp_grpc_client_options.h>
#include <bits/chrono.h>
#include <google/protobuf/arena.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/resource_quota.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/channel_arguments.h>
#include <grpcpp/support/status.h>
#include <opentelemetry/common/macros.h>
#include <opentelemetry/common/timestamp.h>
#include <opentelemetry/ext/http/common/url_parser.h>
#include <opentelemetry/sdk/common/exporter_utils.h>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <ratio>
#include <string>
#include <utility>
#include "common/opentelemetry/export_trace_service_call_data.h"
#include "opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.h"
#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h"
#include "opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h"
#include "opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h"
#include "opentelemetry/proto/collector/trace/v1/trace_service.pb.h"

namespace App {
namespace Common {
namespace Opentelemetry {

using namespace proto::collector::trace::v1;
using namespace proto::collector::metrics::v1;
using namespace proto::collector::logs::v1;

// When building with -fvisibility=default, we hide the symbols and vtable to ensure we always use
// local version of OtlpGrpcAsyncCallDataBase and OtlpGrpcAsyncCallData. It's used to keep
// compatibility when executables links multiple versions of otel-cpp.
class OPENTELEMETRY_LOCAL_SYMBOL OtlpGrpcAsyncCallDataBase {
public:
    using GrpcAsyncCallback = bool (*)(OtlpGrpcAsyncCallDataBase*);

    std::unique_ptr<google::protobuf::Arena> arena;
    grpc::Status grpc_status;
    std::unique_ptr<grpc::ClientContext> grpc_context;

    opentelemetry::sdk::common::ExportResult export_result = opentelemetry::sdk::common::ExportResult::kFailure;
    GrpcAsyncCallback grpc_async_callback = nullptr;

    OtlpGrpcAsyncCallDataBase() = default;
    virtual ~OtlpGrpcAsyncCallDataBase() {
    }
};

// When building with -fvisibility=default, we hide the symbols and vtable to ensure we always use
// local version of OtlpGrpcAsyncCallDataBase and OtlpGrpcAsyncCallData. It's used to keep
// compatibility when executables links multiple versions of otel-cpp.
template <class GrpcRequestType, class GrpcResponseType>
class OPENTELEMETRY_LOCAL_SYMBOL OtlpGrpcAsyncCallData : public OtlpGrpcAsyncCallDataBase {
public:
    using RequestType = GrpcRequestType;
    using ResponseType = GrpcResponseType;

    RequestType* request = nullptr;
    ResponseType* response = nullptr;

    std::function<bool(opentelemetry::sdk::common::ExportResult, std::unique_ptr<google::protobuf::Arena>&&,
                       const RequestType&, ResponseType*)>
        result_callback;

    OtlpGrpcAsyncCallData() {
    }
    virtual ~OtlpGrpcAsyncCallData() = default;
};

struct OtlpGrpcClientAsyncData {
    std::chrono::system_clock::duration export_timeout = std::chrono::seconds{10};

    // The best performance trade-off of gRPC is having numcpu's threads and one completion queue
    // per thread, but this exporter should not cost a lot resource and we don't want to create
    // too many threads in the process. So we use one completion queue.
    grpc::CompletionQueue cq;

    // Running requests, this is used to limit the number of concurrent requests.
    std::atomic<std::size_t> running_requests{0};
    // Request counter is used to record ForceFlush.
    std::atomic<std::size_t> start_request_counter{0};
    std::atomic<std::size_t> finished_request_counter{0};
    std::size_t max_concurrent_requests = 64;

    // Condition variable and mutex to control the concurrency count of running requests.
    std::mutex session_waker_lock;
    std::condition_variable session_waker;

    // Do not use OtlpGrpcClientAsyncData() = default; here, some versions of GCC&Clang have BUGs
    // and may not initialize the member correctly. See also
    // https://stackoverflow.com/questions/53408962/try-to-understand-compiler-error-message-default-member-initializer-required-be
    OtlpGrpcClientAsyncData() = default;
};
// ----------------------------- Helper functions ------------------------------
static std::string GetFileContents(const char* fpath) {
    std::ifstream finstream(fpath);
    std::string contents;
    contents.assign((std::istreambuf_iterator<char>(finstream)), std::istreambuf_iterator<char>());
    finstream.close();
    return contents;
}

// If the file path is non-empty, returns the contents of the file. Otherwise returns contents.
static std::string GetFileContentsOrInMemoryContents(const std::string& file_path, const std::string& contents) {
    if (!file_path.empty()) {
        return GetFileContents(file_path.c_str());
    }
    return contents;
}

OtlpGrpcClient::OtlpGrpcClient(std::shared_ptr<OtlpGrpcClientOptions>& options_, grpc::CompletionQueue* cq)
    : is_shutdown_(false), cq_(cq) {
    options = options_;
}

OtlpGrpcClient::~OtlpGrpcClient() {
    std::shared_ptr<OtlpGrpcClientAsyncData> async_data;
    async_data.swap(async_data_);

    while (async_data && async_data->running_requests.load(std::memory_order_acquire) > 0) {
        std::unique_lock<std::mutex> lock{async_data->session_waker_lock};
        async_data->session_waker.wait_for(lock, async_data->export_timeout, [async_data]() {
            return async_data->running_requests.load(std::memory_order_acquire) == 0;
        });
    }
}

std::shared_ptr<grpc::Channel> OtlpGrpcClient::MakeChannel(const OtlpGrpcClientOptions& options) {
    std::shared_ptr<grpc::Channel> channel;

    //
    // Scheme is allowed in OTLP endpoint definition, but is not allowed for creating gRPC
    // channel. Passing URI with scheme to grpc::CreateChannel could resolve the endpoint to some
    // unexpected address.
    //

    opentelemetry::ext::http::common::UrlParser url(options.endpoint);
    if (!url.success_) {
        //        OTEL_INTERNAL_LOG_ERROR("[OTLP GRPC Client] invalid endpoint: " << options.endpoint);

        return nullptr;
    }

    std::string grpc_target = url.host_ + ":" + std::to_string(static_cast<int>(url.port_));
    grpc::ChannelArguments grpc_arguments;
    grpc_arguments.SetUserAgentPrefix(options.user_agent);

    if (options.max_threads > 0) {
        grpc::ResourceQuota quota;
        quota.SetMaxThreads(static_cast<int>(options.max_threads));
        grpc_arguments.SetResourceQuota(quota);
    }

    if (options.use_ssl_credentials) {
        grpc::SslCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs = GetFileContentsOrInMemoryContents(options.ssl_credentials_cacert_path,
                                                                    options.ssl_credentials_cacert_as_string);
#if ENABLE_OTLP_GRPC_SSL_MTLS_PREVIEW
        ssl_opts.pem_private_key =
            GetFileContentsOrInMemoryContents(options.ssl_client_key_path, options.ssl_client_key_string);
        ssl_opts.pem_cert_chain =
            GetFileContentsOrInMemoryContents(options.ssl_client_cert_path, options.ssl_client_cert_string);

#endif
        channel = grpc::CreateCustomChannel(grpc_target, grpc::SslCredentials(ssl_opts), grpc_arguments);
    } else {
        channel = grpc::CreateCustomChannel(grpc_target, grpc::InsecureChannelCredentials(), grpc_arguments);
    }

    return channel;
}

std::unique_ptr<grpc::ClientContext> OtlpGrpcClient::MakeClientContext(const OtlpGrpcClientOptions& options) {
    auto context = std::make_unique<grpc::ClientContext>();
    if (options.timeout.count() > 0) {
        context->set_deadline(std::chrono::system_clock::now() + options.timeout);
    }

    for (auto& header : options.metadata) {
        context->AddMetadata(header.first, header.second);
    }

    return context;
}

std::unique_ptr<TraceService::Stub> OtlpGrpcClient::MakeTraceServiceStub(const OtlpGrpcClientOptions& options) {
    return TraceService::NewStub(MakeChannel(options));
}

std::unique_ptr<MetricsService::Stub> OtlpGrpcClient::MakeMetricsServiceStub(const OtlpGrpcClientOptions& options) {
    return MetricsService::NewStub(MakeChannel(options));
}

std::unique_ptr<LogsService::Stub> OtlpGrpcClient::MakeLogsServiceStub(const OtlpGrpcClientOptions& options) {
    return LogsService::NewStub(MakeChannel(options));
}

std::shared_ptr<OtlpGrpcClientAsyncData> OtlpGrpcClient::MutableAsyncData(const OtlpGrpcClientOptions& options) {
    if (!async_data_) {
        async_data_ = std::make_shared<OtlpGrpcClientAsyncData>();
        async_data_->export_timeout = options.timeout;
        async_data_->max_concurrent_requests = options.max_concurrent_requests;
    }

    return async_data_;
}

bool OtlpGrpcClient::ForceFlush(std::chrono::microseconds timeout) noexcept {
    if (!async_data_) {
        return true;
    }

    std::size_t request_counter = async_data_->start_request_counter.load(std::memory_order_acquire);
    if (request_counter <= async_data_->finished_request_counter.load(std::memory_order_acquire)) {
        return true;
    }

    // ASAN will report chrono: runtime error: signed integer overflow: A + B cannot be represented
    //   in type 'long int' here. So we reset timeout to meet signed long int limit here.
    timeout = opentelemetry::common::DurationUtil::AdjustWaitForTimeout(timeout, std::chrono::microseconds::zero());

    // Wait for all the sessions to finish
    std::unique_lock<std::mutex> lock(async_data_->session_waker_lock);

    std::chrono::steady_clock::duration timeout_steady =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(timeout);
    if (timeout_steady <= std::chrono::steady_clock::duration::zero()) {
        timeout_steady = std::chrono::steady_clock::duration::max();
    }

    while (timeout_steady > std::chrono::steady_clock::duration::zero() &&
           request_counter > async_data_->finished_request_counter.load(std::memory_order_acquire)) {
        // When changes of running_sessions_ and notify_one/notify_all happen between predicate
        // checking and waiting, we should not wait forever.We should cleanup gc sessions here as soon
        // as possible to call FinishSession() and cleanup resources.
        std::chrono::steady_clock::time_point start_timepoint = std::chrono::steady_clock::now();
        if (std::cv_status::timeout != async_data_->session_waker.wait_for(lock, async_data_->export_timeout)) {
            break;
        }

        timeout_steady -= std::chrono::steady_clock::now() - start_timepoint;
    }

    return timeout_steady > std::chrono::steady_clock::duration::zero();
}

bool OtlpGrpcClient::Shutdown(std::chrono::microseconds timeout) noexcept {
    if (!async_data_) {
        return true;
    }

    if (!is_shutdown_.exchange(true, std::memory_order_acq_rel)) {
        is_shutdown_ = true;

        async_data_->cq.Shutdown();
    }

    return ForceFlush(timeout);
}

std::shared_ptr<OtlpGrpcClientAsyncData> OtlpGrpcClient::checkRunStatus() {
    if (is_shutdown_.load(std::memory_order_acquire)) {
        //        OTEL_INTERNAL_LOG_ERROR("[OTLP GRPC Client] ERROR: Export "
        //                                        << span_count << " trace span(s) failed, exporter is shutdown");
        //        if (result_callback)
        //        {
        //            result_callback(opentelemetry::sdk::common::ExportResult::kFailure, std::move(arena), request,
        //                            nullptr);
        //        }
        return nullptr;
    }

    auto async_data = MutableAsyncData(*options);
    if (async_data->running_requests.load(std::memory_order_acquire) >= async_data->max_concurrent_requests) {
        //        OTEL_INTERNAL_LOG_ERROR("[OTLP GRPC Client] ERROR: Export "
        //                                        << export_data_count << " " << export_data_name
        //                                        << " failed, exporter queue is full");
        //        if (result_callback)
        //        {
        //            result_callback(opentelemetry::sdk::common::ExportResult::kFailureFull, std::move(arena),
        //                            request, nullptr);
        //        }
        return nullptr;
    }
    return async_data;
}

sdk::common::ExportResult OtlpGrpcClient::Export(
    std::unique_ptr<TraceService::Stub>& stub, ExportTraceServiceRequest&& request,
    std::unique_ptr<google::protobuf::Arena>&& arena,
    OpenTelemetry::ExportTraceServiceCallData<ExportTraceServiceResponse>::AsyncCallable asyncCallable) {
    auto async_data = checkRunStatus();
    if (!async_data) {
        return opentelemetry::sdk::common::ExportResult::kFailure;
    }

    auto asyncCall = new OpenTelemetry::ExportTraceServiceCallData<ExportTraceServiceResponse>(cq_);
    ++async_data->start_request_counter;
    ++async_data->running_requests;
    asyncCall->setArena(std::move(arena));
    asyncCall->setAsyncCallable(asyncCallable);
    asyncCall->setCallable([async_data, asyncCall](const grpc::Status&) {
        --async_data->running_requests;
        ++async_data->finished_request_counter;

        if (auto func = asyncCall->getAsyncCallback(); func != nullptr) {
            func(asyncCall);
        }
        return;
    });

    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    auto requestData =
        google::protobuf::Arena::Create<ExportTraceServiceRequest>(asyncCall->getArena().get(), std::move(request));
    asyncCall->setResponseReader(stub->PrepareAsyncExport(&asyncCall->getContext(), *requestData, cq_));
    asyncCall->startCall();
    asyncCall->finishCall();
    return opentelemetry::sdk::common::ExportResult::kSuccess;
}

sdk::common::ExportResult OtlpGrpcClient::Export(
    std::unique_ptr<MetricsService::Stub>& stub, ExportMetricsServiceRequest&& request,
    std::unique_ptr<google::protobuf::Arena>&& arena,
    OpenTelemetry::ExportTraceServiceCallData<ExportMetricsServiceResponse>::AsyncCallable asyncCallable) {
    auto async_data = checkRunStatus();
    if (!async_data) {
        return opentelemetry::sdk::common::ExportResult::kFailure;
    }

    auto asyncCall = new OpenTelemetry::ExportTraceServiceCallData<ExportMetricsServiceResponse>(cq_);
    ++async_data->start_request_counter;
    ++async_data->running_requests;
    asyncCall->setArena(std::move(arena));
    asyncCall->setAsyncCallable(asyncCallable);
    asyncCall->setCallable([async_data, asyncCall](const grpc::Status& ) {
        --async_data->running_requests;
        ++async_data->finished_request_counter;

        if (auto func = asyncCall->getAsyncCallback(); func != nullptr) {
            func(asyncCall);
        }
        return;
    });

    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    auto requestData =
        google::protobuf::Arena::Create<ExportMetricsServiceRequest>(asyncCall->getArena().get(), std::move(request));
    asyncCall->setResponseReader(stub->PrepareAsyncExport(&asyncCall->getContext(), *requestData, cq_));
    asyncCall->startCall();
    asyncCall->finishCall();
    return opentelemetry::sdk::common::ExportResult::kSuccess;
}

sdk::common::ExportResult OtlpGrpcClient::Export(
    std::unique_ptr<LogsService::Stub>& stub, ExportLogsServiceRequest&& request,
    std::unique_ptr<google::protobuf::Arena>&& arena,
    OpenTelemetry::ExportTraceServiceCallData<ExportLogsServiceResponse>::AsyncCallable asyncCallable) {
    auto async_data = checkRunStatus();
    if (!async_data) {
        return opentelemetry::sdk::common::ExportResult::kFailure;
    }

    auto asyncCall = new OpenTelemetry::ExportTraceServiceCallData<ExportLogsServiceResponse>(cq_);
    ++async_data->start_request_counter;
    ++async_data->running_requests;
    asyncCall->setArena(std::move(arena));
    asyncCall->setAsyncCallable(asyncCallable);
    asyncCall->setCallable([async_data, asyncCall](const grpc::Status&) {
        --async_data->running_requests;
        ++async_data->finished_request_counter;

        if (auto func = asyncCall->getAsyncCallback(); func != nullptr) {
            func(asyncCall);
        }
        return;
    });

    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    auto requestData =
        google::protobuf::Arena::Create<ExportLogsServiceRequest>(asyncCall->getArena().get(), std::move(request));
    asyncCall->setResponseReader(stub->PrepareAsyncExport(&asyncCall->getContext(), *requestData, cq_));
    asyncCall->startCall();
    asyncCall->finishCall();
    return opentelemetry::sdk::common::ExportResult::kSuccess;
}
} // namespace Opentelemetry
} // namespace Common
} // namespace App
