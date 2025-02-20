//
// Created by zhanglei on 2025/2/20.
//
#include "app/include/sink/topology/sink.h"

#include <spdlog/spdlog.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/client_callback.h>

namespace App::Sink::Topology {
using namespace novaagent::trace::v1;
/**
 * Client-> Server的单向的请求
 * @tparam RequestType
 * @tparam ResponseType
 */
template <typename RequestType, typename ResponseType> //
struct AsyncUnaryCall : public grpc::ClientUnaryReactor {
    grpc::Status status;
    grpc::ClientContext context;
    RequestType request{};
    ResponseType response{};
    std::function<void(const grpc::Status&, const ResponseType&)> callback;
    explicit AsyncUnaryCall(const App::Common::Grpc::Headers& headers) {
        for (const auto& [key, value] : headers) {
            context.AddMetadata(key, value);
            SPDLOG_TRACE("add grpc header: {}={}", key, value);
        }
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
    }

    void OnDone(const grpc::Status& s) override {
        if (callback) {
            callback(s, response);
        }
    }
};


Sink::Sink(Common::Grpc::ClientOptions options) :options_(options){
    SPDLOG_INFO("topology endpoint is: {}", options.endpoint);
    stub_ = std::make_unique<novaagent::trace::v1::TraceTopologyCollectorService::Stub>(
        grpc::CreateChannel(options_.endpoint, grpc::InsecureChannelCredentials()));
    max_concurrent_requests = options.max_concurrent_requests;
}

void Sink::registerService(novaagent::trace::v1::Service& service) {
    if (running_requests.load(std::memory_order_acquire) > max_concurrent_requests) {
        SPDLOG_DEBUG("registerService full, max_concurrent_requests", max_concurrent_requests);
        return;
    }
    ++running_requests;
    auto* call = new AsyncUnaryCall<novaagent::trace::v1::Service, novaagent::trace::v1::TopologyRes>(options_.metadata);
    call->callback = [call, this](const grpc::Status& status, const TopologyRes& resp) {
        if (!status.ok()) {
            SPDLOG_ERROR("registerService error: {}", status.error_message());
        }
        delete (call);
        --running_requests;
    };
    call->request.Swap(&service);
    SPDLOG_DEBUG("registerService request: {}", call->request.ShortDebugString());
    stub_->async()->RegisterService(&call->context, &call->request, &call->response, call);
    call->StartCall();
}

void Sink::registerServiceRelation(novaagent::trace::v1::ServiceRelation &relation) {
    if (running_requests.load(std::memory_order_acquire) > max_concurrent_requests) {
        SPDLOG_DEBUG("registerService full, max_concurrent_requests", max_concurrent_requests);
        return;
    }
    ++running_requests;
    auto* call = new AsyncUnaryCall<novaagent::trace::v1::ServiceRelation, novaagent::trace::v1::TopologyRes>(options_.metadata);
    call->callback = [call, this](const grpc::Status& status, const TopologyRes& resp) {
        if (!status.ok()) {
            SPDLOG_ERROR("registerServiceRelation error: {}", status.error_message());
        }
        delete (call);
        --running_requests;
    };
    call->request.Swap(&relation);
    SPDLOG_DEBUG("registerServiceRelation request: {}", call->request.ShortDebugString());
    stub_->async()->RegisterServiceRelation(&call->context, &call->request, &call->response, call);
    call->StartCall();
}
}