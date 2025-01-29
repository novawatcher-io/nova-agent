#pragma once
#include "app/include/common/const.h"
#include "collector/api/collector.h"
#include "config/nova_agent_config.h"
#include "node/v1/info.pb.h"
#include "sink/deep_observe/node/sink.h"
#include "source/host/collector/cgroup/cgroup_collector.h"
#include "source/host/collector/container/container_collector.h"
#include "source/host/collector/container/docker_cri_client.h"
#include "source/host/collector/cpu/cpu_topology.h"
#include "source/host/collector/cpu/cpu_usage.h"
#include "source/host/collector/disk/disk_collector.h"
#include "source/host/collector/gpu/gpu_collector.h"
#include "source/host/collector/oltp/oltp.h"
#include "source/host/collector/process/proc_reader.h"
#include <component/api.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Core::Component {
class TimerChannel;
class UnixThreadContainer;
} // namespace Core::Component

namespace Core::Event {
class EventLoop;
}

namespace grpc {
class CompletionQueue;
}

namespace App::Source::Host {
class Source : public Core::Component::Component {
public:
    Source(const std::shared_ptr<Core::Event::EventLoop>& loop_,
           const std::shared_ptr<Core::Component::UnixThreadContainer>& threadPool_,
           const std::shared_ptr<App::Config::ConfigReader>& config);

    std::string name() {
        return App::Common::Trace::Skywalking::skywalking_grpc;
    }

    void init() final;

    void start() final;

    void stop() final;

    void finish() final;

private:
    const std::shared_ptr<Core::Component::UnixThreadContainer>& threadPool;

    const std::shared_ptr<Core::Event::EventLoop>& loop;

    std::shared_ptr<Core::Component::TimerChannel> timer_;

    // 派遣线程计数器
    uint8_t count = 0;

    App::Source::Host::Collector::Cpu::CPUTopology cpu_topo_;
    std::unique_ptr<App::Source::Host::Collector::Process::ProcReader> proc_reader_;
    std::unique_ptr<App::Source::Host::Collector::Cpu::CpuUsage> cpu_collector_;
    std::unique_ptr<App::Source::Host::Collector::GPU::GPUCollector> gpu_collector_;
    std::unique_ptr<App::Source::Host::Collector::Oltp::OltpCollector> oltp_collector_;
    std::unique_ptr<App::Source::Host::Collector::Container::ContainerCollector> container_collector_;
    std::unique_ptr<App::Source::Host::Collector::Container::DockerCRIClient> cri_collector_;
    std::unique_ptr<App::Source::Host::Collector::Disk::DiskCollector> disk_collector_;
    std::unique_ptr<App::Source::Host::Collector::CGroup::CGroupCollector> cgroup_collector_;
    std::vector<std::unique_ptr<Collector::Api::Collector>> collectors;

    // 创建sink
    std::unique_ptr<Sink::DeepObserve::Node::Sink> sink;

    novaagent::node::v1::NodeInfo exportInfo;
    std::shared_ptr<App::Config::ConfigReader> config_;
};
} // namespace App::Source::Host
