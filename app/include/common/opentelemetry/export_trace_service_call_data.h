//
// Created by root on 2024/4/5.
//

#pragma once
#include "app/include/api/call_data.h"
#include <opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>

using namespace opentelemetry::proto::collector::trace::v1;
namespace App {
namespace Common {
namespace OpenTelemetry {
template <class Response> class ExportTraceServiceCallData : public Api::CallData {
public:
    using AsyncCallable = bool (*)(ExportTraceServiceCallData*);

    explicit ExportTraceServiceCallData(grpc::CompletionQueue* cq) : cq_(cq) {
        Proceed();
    }

    void onCreate() final {
    }

    void onProcess() final {
        callable(status);
        delete this;
    }

    void onFinish() final {
    }

    grpc::ClientContext& getContext() {
        return context;
    }

    void setResponseReader(std::unique_ptr<grpc::ClientAsyncResponseReader<Response>> response_reader_) {
        response_reader = std::move(response_reader_);
    }

    void startCall() {
        response_reader->StartCall();
    }

    void finishCall() {
        response_reader->Finish(&reply, &status, (void*)this);
    }

    void setCallable(std::function<void(grpc::Status)> callable_) {
        callable = std::move(callable_);
    }

    void setArena(std::unique_ptr<google::protobuf::Arena>&& arena_) {
        arena.swap(arena_);
    }

    AsyncCallable getAsyncCallback() {
        return grpc_async_callback;
    }

    grpc::Status& getStatus() {
        return status;
    }

    std::unique_ptr<google::protobuf::Arena>& getArena() {
        return arena;
    }

    void setAsyncCallable(AsyncCallable grpc_async_callback_) {
        grpc_async_callback = grpc_async_callback_;
    }

    ~ExportTraceServiceCallData() override {
    }

private:
    // Storage for the status of the RPC upon completion.
    grpc::Status status;

    AsyncCallable grpc_async_callback = nullptr;
    // Container for the data we expect from the server.
    Response reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    grpc::ClientContext context;

    std::unique_ptr<grpc::ClientAsyncResponseReader<Response>> response_reader;
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::CompletionQueue* cq_;

    std::unique_ptr<TraceService::Stub> stub_;

    std::function<void(grpc::Status)> callable;

    std::unique_ptr<google::protobuf::Arena> arena;
};
} // namespace OpenTelemetry
} // namespace Common
} // namespace App