#pragma once
#include "app/include/source/host/collector/container/docker_http_client.h"
#include "app/include/source/host/collector/process/proc_reader.h"
#include "nova_agent_payload/node/v1/info.pb.h"
#include <cassert>
#include <chrono>
#include <component/timer_channel.h>
#include <event/event_buffer_channel.h>
#include <event/event_loop.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace App::Source::Host::Collector::Container {
using ::novaagent::node::v1::ContainerInfoRequest;
class ContainerCollector {
public:
    explicit ContainerCollector(Core::Event::EventLoop* loop, const std::string& unix_socket) {
        if (unix_socket.empty()) {
            SPDLOG_INFO("unix domain socket is empty, docker container will not be collected");
            return;
        }
        client_ = std::make_unique<DockerHttpClient>(
            loop->getEventBase(), unix_socket,
            [this](std::vector<ContainerInfo> container_list) { OnContainerList(container_list); });
        assert(loop != nullptr);
        timer_ = std::make_unique<Core::Component::TimerChannel>(loop, [this]() {
            GetContainerListAsync();
            timer_->enable(std::chrono::seconds(3));
        });
        timer_->enable(std::chrono::seconds(3));
    }

    void GetContainerList(ContainerInfoRequest& request,
                          const std::unordered_map<int, Process::ProcessSampleInfo>& table) const;

    void OnContainerList(std::vector<ContainerInfo>& container_list);

private:
    void GetContainerListAsync() const;

    std::unique_ptr<DockerHttpClient> client_;
    std::vector<ContainerInfo> container_list_;
    std::unique_ptr<Core::Component::TimerChannel> timer_;
    mutable std::mutex request_mutex_;
    ContainerInfoRequest request_;
};
} // namespace App::Source::Host::Collector::Container
