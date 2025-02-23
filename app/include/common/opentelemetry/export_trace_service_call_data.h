//
// Created by root on 2024/4/5.
//

#pragma once
#include <google/protobuf/arena.h>
#include <opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>

using namespace opentelemetry::proto::collector::trace::v1;
namespace App {
namespace Common {
namespace OpenTelemetry {
template <typename RequestType, typename ResponseType>
class ExportTraceServiceCallData : public grpc::ClientUnaryReactor {
public:
    grpc::Status status;
    grpc::ClientContext context;
    RequestType request{};
    ResponseType response{};
//    using AsyncCallable = std::function<void(const grpc::Status&, const ResponseType&)>;

    ExportTraceServiceCallData()  {
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
    }

    grpc::ClientContext& getContext() {
        return context;
    }


    void setCallable(std::function<void(const grpc::Status&, const ResponseType&)> callable_) {
        callable = std::move(callable_);
    }

    std::function<void(const grpc::Status&, const ResponseType&)> getAsyncCallback() {
        return grpc_async_callback;
    }

    void setAsyncCallable(std::function<void(const grpc::Status&, const ResponseType&)> grpc_async_callback_) {
        grpc_async_callback = grpc_async_callback_;
    }

    void OnDone(const grpc::Status& s) override {
        if (callable) {
            callable(s, response);
        }
    }

    ~ExportTraceServiceCallData() override {
    }

private:
    std::function<void(const grpc::Status&, const ResponseType&)> grpc_async_callback = nullptr;
    std::function<void(const grpc::Status&, const ResponseType&)> callable;
};
} // namespace OpenTelemetry
} // namespace Common
} // namespace App