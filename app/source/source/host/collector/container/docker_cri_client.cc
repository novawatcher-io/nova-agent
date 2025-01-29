#include "source/host/collector/container/docker_cri_client.h"
#include "common/grpc/grpc_client_options.h"
#include "cri-api/api.grpc.pb.h"
#include "cri-api/api.pb.h"
#include "nova_agent_payload/node/v1/info.pb.h"
#include <any>
#include <cassert>
#include <component/timer_channel.h>
#include <functional>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/status.h>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <string>

using ::novaagent::node::v1::ContainerInfoRequest;
using runtime::v1::Container;
using runtime::v1::ContainerStatsRequest;
using runtime::v1::ContainerStatsResponse;
using runtime::v1::ListContainersRequest;
using runtime::v1::ListContainersResponse;
using runtime::v1::ListPodSandboxRequest;
using runtime::v1::ListPodSandboxResponse;
using runtime::v1::PodSandbox;
using runtime::v1::RuntimeService;

namespace App::Source::Host::Collector::Container {
/**
 * todo: need to move to common
 *
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
    std::any private_data;

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

static bool IsUnixDomainSocket(const char* path) {
    struct stat statbuf{};
    if (stat(path, &statbuf) != 0) {
        return false;
    }
    return S_ISSOCK(statbuf.st_mode);
}

DockerCRIClient::DockerCRIClient(Core::Event::EventLoop* loop, const std::string& api_server_address) {
    if (api_server_address.empty()) {
        SPDLOG_INFO("CRI API server address is empty");
        return;
    }

    if (!IsUnixDomainSocket(api_server_address.c_str())) {
        SPDLOG_INFO("invalid unix domain socket: {}", api_server_address);
        return;
    }
    server_address_ = "unix:" + api_server_address;
    SPDLOG_INFO("cri server address: {}", server_address_);

    auto channel = grpc::CreateChannel(server_address_, grpc::InsecureChannelCredentials());

    stub_ = RuntimeService::NewStub(channel);
    assert(loop != nullptr);
    timer_ = std::make_unique<Core::Component::TimerChannel>(loop, [this]() {
        {
            std::lock_guard<std::mutex> const guard(request_mutex_);
            request_.Clear();
        }
        AsyncGetContainerList();
        timer_->enable(std::chrono::seconds(3));
    });
    timer_->enable(std::chrono::seconds(3));
}

void DockerCRIClient::AsyncGetContainerList() {
    auto* call = new AsyncUnaryCall<ListContainersRequest, ListContainersResponse>(options_.metadata);
    call->callback = [&](const grpc::Status& status, const ListContainersResponse& resp) {
        if (status.ok()) {
            SPDLOG_DEBUG("resp {}", resp.ShortDebugString());
            OnListContainersResp(resp);
        } else {
            SPDLOG_ERROR("error: {}", status.error_message());
        }
    };
    SPDLOG_DEBUG("ReportContainer request: {}", call->request.ShortDebugString());
    stub_->async()->ListContainers(&call->context, &call->request, &call->response, call);
    call->StartCall();
}

void DockerCRIClient::OnListContainersResp(const ListContainersResponse& response) {
    for (const auto& container : response.containers()) {
        SPDLOG_DEBUG("container info: {}", container.ShortDebugString());
        auto* item = new ::novaagent::node::v1::ContainerInfo;
        item->set_id(container.id());
        switch (container.state()) {
        case ::runtime::v1::ContainerState::CONTAINER_CREATED:
            item->set_state("created");
            break;
        case ::runtime::v1::ContainerState::CONTAINER_EXITED:
            item->set_state("exited");
            break;
        case ::runtime::v1::ContainerState::CONTAINER_RUNNING:
            item->set_state("running");
            break;
        case ::runtime::v1::ContainerState::CONTAINER_UNKNOWN:
            // fallthrough
        default:
            item->set_state("unknown");
        }
        item->set_image_name(container.image().image());
        item->set_start_time(container.created_at());
        AsyncGetContainerInfo(item);
    }
}

void DockerCRIClient::AsyncGetContainerInfo(::novaagent::node::v1::ContainerInfo* item) {
    auto* call = new AsyncUnaryCall<ContainerStatsRequest, ContainerStatsResponse>(options_.metadata);
    call->callback = [this, call](const grpc::Status& status, const ContainerStatsResponse& resp) {
        auto info = std::any_cast<::novaagent::node::v1::ContainerInfo*>(call->private_data);
        if (status.ok()) {
            if (info != nullptr) {
                SPDLOG_DEBUG("ContainerStats response: {}", resp.ShortDebugString());
                OnContainerInfoResp(info, resp);
            } else {
                SPDLOG_ERROR("null item found");
            }
        } else {
            SPDLOG_ERROR("get container info failed, container_id={}, error={}", info->id(), status.error_message());
        }
    };
    call->request.set_container_id(item->id());
    call->private_data = item;
    stub_->async()->ContainerStats(&call->context, &call->request, &call->response, call);
    call->StartCall();
}

void DockerCRIClient::OnContainerInfoResp(::novaagent::node::v1::ContainerInfo* item,
                                          const runtime::v1::ContainerStatsResponse& resp) {
    item->set_memory_rss(resp.stats().memory().rss_bytes().value());
    item->set_memory_limit(resp.stats().memory().working_set_bytes().value());
    {
        std::lock_guard<std::mutex> const guard(request_mutex_);
        request_.mutable_containers()->AddAllocated(item);
    }
}

void DockerCRIClient::GetContainerList(ContainerInfoRequest& request) {
    std::lock_guard<std::mutex> const guard(request_mutex_);
    request.Swap(&request_);
    request_.Clear();
}
} // namespace App::Source::Host::Collector::Container
