#pragma once
#include "app/include/api/call_data.h"
#include <google/protobuf/arena.h>

namespace App {
namespace Sink {
namespace DeepObserve {
namespace Node {
template <class Response> class CallData :public Api::CallData {
public:
    using AsyncCallable = bool (*)(CallData*);
    explicit CallData(grpc::CompletionQueue*  /*cq_*/) {
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

    void setCallable(std::function<void(grpc::Status)> callable_) {
        callable = std::move(callable_);
    }

    void setArena(std::unique_ptr<google::protobuf::Arena>&& arena_) {
        arena.swap(arena_);
    }

    void setAsyncCallable(AsyncCallable grpc_async_callback_) {
        grpc_async_callback = grpc_async_callback_;
    }

    AsyncCallable getAsyncCallback() {
        return grpc_async_callback;
    }

    void setResponseReader(std::unique_ptr<grpc::ClientAsyncResponseReader<Response>> response_reader_) {
        response_reader = std::move(response_reader_);
    }

    void startCall() {
        response_reader->StartCall();
    }

    grpc::ClientContext& getContext() {
        return context;
    }

    std::unique_ptr<google::protobuf::Arena>& getArena() {
        return arena;
    }

    void finishCall() {
        response_reader->Finish(&reply, &status, (void*)this);
    }


private:
    std::function<void(grpc::Status)> callable;
    std::unique_ptr<google::protobuf::Arena> arena;
    AsyncCallable grpc_async_callback = nullptr;
    std::unique_ptr<grpc::ClientAsyncResponseReader<Response>> response_reader;
    // Container for the data we expect from the server.
    Response reply;
    // Storage for the status of the RPC upon completion.
    grpc::Status status;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    grpc::ClientContext context;
};
}
}
}
}