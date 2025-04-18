#include "app/include/source/host/source.h"
#include "app/include/common/machine.h"
#include "app/include/common/const.h"
#include "app/include/source/host/collector/cpu/collector.h"
#include "app/include/source/host/collector/gpu/gpu_collector.h"
#include "app/include/source/host/collector/node/collector.h"
#include "app/include/source/host/collector/fs/fs_collector.h"
#include "component/thread_container.h"
#include "component/timer_channel.h"
#include "config/nova_agent_config.h"
#include "proto/nova_agent_config.pb.h"
#include "sink/deep_observe/node/sink.h"
#include "source/host/collector/api/collector.h"
#include <chrono>
#include <memory>
#include <spdlog/spdlog.h>
#include <utility>

namespace Core::Event {
class EventLoop;
}

namespace grpc {
class CompletionQueue;
}
using ::novaagent::node::v1::ContainerInfoRequest;

namespace App::Source::Host {
Source::Source(const std::shared_ptr<Core::Event::EventLoop>& loop_,
               const std::shared_ptr<Core::Component::UnixThreadContainer>& threadPool_,
               const std::shared_ptr<App::Config::ConfigReader>& config)
    : threadPool(threadPool_), loop(loop_), config_(config) {
    sink = std::make_unique<Sink::DeepObserve::Node::Sink>(config);
    exportInfo.set_company_uuid(config_->GetConfig().company_uuid());
    exportInfo.set_cluster(config_->GetConfig().cluster());
    proc_reader_ = std::make_unique<Host::Collector::Process::ProcReader>();
    gpu_collector_ = std::make_unique<Host::Collector::GPU::GPUCollector>(config_->GetConfig().company_uuid());
    if (config_->GetConfig().has_container_collector_config()) {
        if (config->GetConfig().container_collector_config().enable()) {
            if (config->GetConfig().container_collector_config().container_runtime() == App::Common::Container::docker) {
                container_collector_ = std::make_unique<Host::Collector::Container::ContainerCollector>(
                                       loop.get(), config_->GetConfig().container_collector_config().container_unix_domain_socket());
            } else {
                cri_collector_ = std::make_unique<Host::Collector::Container::DockerCRIClient>(
                   loop.get(), config_->GetConfig().container_collector_config().container_unix_domain_socket());
            }
        }
    }
    disk_collector_ = std::make_unique<Collector::Disk::DiskCollector>(config_->GetConfig().company_uuid());
    oltp_collector_ = std::make_unique<App::Source::Host::Collector::Oltp::OltpCollector>(proc_reader_.get());
    cpu_collector_ = std::make_unique<App::Source::Host::Collector::Cpu::CpuUsage>(config_->GetConfig().company_uuid(),
                                                                          proc_reader_.get());
    fs_collector_ = std::make_unique<Collector::Fs::FsCollector>();
    cgroup_collector_ = std::make_unique<Collector::CGroup::CGroupCollector>();
    network_collector_ = std::make_unique<Collector::Network::NetworkCollector>();
    node_collector_ = std::make_unique<Collector::Node::Collector>();
    iostat_collector_ = std::make_unique<Collector::IOStat::IOStatCollector>();
    iostat_collector_ptr_ = iostat_collector_.get();
    node_collector_ptr_ = node_collector_.get();
}

static std::string GetHostname() {
    char hostname[256] = {0};
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return hostname;
    }
    return "";
}

void Source::init() {
    std::unique_ptr<Collector::Api::Collector> collector = std::make_unique<Collector::Cpu::Collector>();
    collectors.push_back(std::move(collector));
    collectors.push_back(std::move(node_collector_));
    collectors.push_back(std::move(iostat_collector_));

    network_collector_->install(&exportInfo);
    network_collector_->install(&exportInfo);

    // collect basic info
    int index = static_cast<int>(count % threadPool->getContainer().size());
    threadPool->task(index, [this] {
        for (auto& iter : collectors) {
            iter->run(&exportInfo);
        }
        cpu_topo_.GetCPUTopology(*exportInfo.mutable_cpu_topology());
        cgroup_collector_->GetCGroupInfo(*exportInfo.mutable_cgroup_info());
        sink->send(exportInfo);
        cpu_collector_->Start();
        return;
    });

    // 初始化采集器列表
    timer_ = std::make_shared<Core::Component::TimerChannel>(loop, [this] {
        if (threadPool->getContainer().empty()) {
            return;
        }

        int index = static_cast<int>(count % threadPool->getContainer().size());
        // todo: report usage info
        threadPool->task(index, [this] {
            SPDLOG_DEBUG("start send metric data");
            novaagent::node::v1::ProcessInfoRequest request;
            novaagent::node::v1::NodeInfo nodeUsageRequest;
            nodeUsageRequest.set_company_uuid(config_->GetConfig().company_uuid());
            nodeUsageRequest.set_cluster(config_->GetConfig().cluster());
            nodeUsageRequest.set_host_name(node_collector_ptr_->host_name_);
            nodeUsageRequest.set_host_object_id(node_collector_ptr_->host_object_id_);
            proc_reader_->GetProcList(&request);
            disk_collector_->GetDiskList(&nodeUsageRequest);
            network_collector_->run(&nodeUsageRequest);
            proc_reader_->GetMemoryInfo(nodeUsageRequest.mutable_virtual_memory_info());
            fs_collector_->run(&nodeUsageRequest);
            iostat_collector_ptr_->collect(&nodeUsageRequest);
            sink->send(nodeUsageRequest);

            auto now = std::chrono::high_resolution_clock::now();
            auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
            request.set_series_id(now_ns);
            auto host_object_id = Common::getMachineId();
            request.set_host_object_id(host_object_id);
            auto hostname = GetHostname();
            request.set_hostname(hostname);
            sink->SendUpdate(request);

            // 容器采集任务
            if (container_collector_) {
                ContainerInfoRequest container_request;
                container_collector_->GetContainerList(container_request, proc_reader_->GetProcessTable());
                container_request.set_series_id(now_ns);
                container_request.set_host_object_id(host_object_id);
                container_request.set_hostname(hostname);
                sink->SendContainerInfo(container_request);
            }
            if (cri_collector_) {
                ContainerInfoRequest k8s_request;
                cri_collector_->GetContainerList(k8s_request);
                if (k8s_request.containers_size() > 0) {
                    k8s_request.set_series_id(now_ns);
                    k8s_request.set_host_object_id(host_object_id);
                    k8s_request.set_hostname(hostname);
                    sink->SendContainerInfo(k8s_request);
                }
            }
            timer_->enable(std::chrono::seconds(10));
        });
    });

}

void Source::start() {
    gpu_collector_->Start();
    oltp_collector_->Start();
    timer_->enable(std::chrono::seconds(10 ));
    iostat_collector_ptr_->start();
    network_collector_->start();
    SPDLOG_INFO("host source start");
}

void Source::stop() {
    timer_->disable();
    gpu_collector_->Stop();
    oltp_collector_->Stop();
    cpu_collector_->Stop();
    iostat_collector_ptr_->stop();
    network_collector_->stop();
    SPDLOG_INFO("host source stop");
}

void Source::finish() {
}

} // namespace App::Source::Host
