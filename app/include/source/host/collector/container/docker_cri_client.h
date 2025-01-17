#pragma once
#include "common/grpc/grpc_client_options.h"
#include "cri-api/api.grpc.pb.h"
#include "deep_agent_payload/node/v1/info.pb.h"
#include <component/timer_channel.h>
#include <event/event_loop.h>
#include <memory>
#include <string>

namespace App::Source::Host::Collector::Container {
using ::deepagent::node::v1::ContainerInfoRequest;
class DockerCRIClient {
public:
    explicit DockerCRIClient(Core::Event::EventLoop* loop, const std::string& api_server_address);

    void GetContainerList(ContainerInfoRequest& request);

private:
    void AsyncGetContainerList();
    void OnListContainersResp(const runtime::v1::ListContainersResponse& response);

    void AsyncGetContainerInfo(::deepagent::node::v1::ContainerInfo* item);
    void OnContainerInfoResp(::deepagent::node::v1::ContainerInfo* item,
                             const runtime::v1::ContainerStatsResponse& response);

    std::string server_address_;
    Common::Grpc::ClientOptions options_;
    std::unique_ptr<runtime::v1::RuntimeService::Stub> stub_;
    std::unique_ptr<Core::Component::TimerChannel> timer_;
    mutable std::mutex request_mutex_;
    ContainerInfoRequest request_;
};

} // namespace App::Source::Host::Collector::Container
