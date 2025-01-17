#include "app/include/sink/deep_observe/node/client.h"
#include "common/grpc/grpc_client_options.h"
#include "node/v1/info.grpc.pb.h"
#include "node/v1/info.pb.h"
#include <bits/chrono.h>
#include <functional>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/status.h>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>

using deepagent::node::v1::ContainerInfoRequest;
using deepagent::node::v1::NodeInfo;
using deepagent::node::v1::NodeRes;
using deepagent::node::v1::NodeUsage;
using deepagent::node::v1::ProcessInfoRequest;

namespace App::Sink::DeepObserve::Node {
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
            SPDLOG_INFO("add grpc header: {}={}", key, value);
        }
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
    }

    void OnDone(const grpc::Status& s) override {
        if (callback) {
            callback(s, response);
        }
    }
};

Client::Client(Common::Grpc::ClientOptions options) : options_(std::move(options)) {
    SPDLOG_INFO("endpoint is: {}", options_.endpoint);
    stub_ = std::make_unique<deepagent::node::v1::NodeCollectorService::Stub>(
        grpc::CreateChannel(options_.endpoint, grpc::InsecureChannelCredentials()));
}

void Client::SendRegisterRequest(deepagent::node::v1::NodeInfo& request) {
    auto* call = new AsyncUnaryCall<NodeInfo, NodeRes>(options_.metadata);
    call->callback = [](const grpc::Status& status, const NodeRes& resp) {
        if (!status.ok()) {
            SPDLOG_ERROR("error: {}", status.error_message());
        }
    };
    call->request.Swap(&request);
    SPDLOG_INFO("Register request: {}", call->request.ShortDebugString());
    stub_->async()->Register(&call->context, &call->request, &call->response, call);
    call->StartCall();
}

void Client::SendUpdateRequest(deepagent::node::v1::NodeUsage& request) {
    auto* call = new AsyncUnaryCall<NodeUsage, NodeRes>(options_.metadata);
    call->callback = [](const grpc::Status& status, const NodeRes& resp) {
        if (!status.ok()) {
            SPDLOG_ERROR("error: {}", status.error_message());
        }
    };
    call->request.Swap(&request);
    SPDLOG_INFO("Update request: {}", call->request.ShortDebugString());
    stub_->async()->Update(&call->context, &call->request, &call->response, call);
    call->StartCall();
}

void Client::SendReportProcessRequest(deepagent::node::v1::ProcessInfoRequest& request) {
    auto* call = new AsyncUnaryCall<ProcessInfoRequest, NodeRes>(options_.metadata);
    call->callback = [](const grpc::Status& status, const NodeRes& resp) {
        if (!status.ok()) {
            SPDLOG_ERROR("error: {}", status.error_message());
        }
    };
    call->request.Swap(&request);
    SPDLOG_INFO("ReportProcess request: {}", call->request.ShortDebugString());
    stub_->async()->ReportProcess(&call->context, &call->request, &call->response, call);
    call->StartCall();
}

void Client::SendReportContainerRequest(deepagent::node::v1::ContainerInfoRequest& request) {
    auto* call = new AsyncUnaryCall<ContainerInfoRequest, NodeRes>(options_.metadata);
    call->callback = [](const grpc::Status& status, const NodeRes& resp) {
        if (!status.ok()) {
            SPDLOG_ERROR("error: {}", status.error_message());
        }
    };
    call->request.Swap(&request);
    SPDLOG_INFO("ReportContainer request: {}", call->request.ShortDebugString());
    stub_->async()->ReportContainer(&call->context, &call->request, &call->response, call);
    call->StartCall();
}

} // namespace App::Sink::DeepObserve::Node
