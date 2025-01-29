#include "app/include/source/host/collector/container/container_collector.h"
#include "nova_agent_payload/node/v1/info.pb.h"
#include <cassert>
#include <spdlog/spdlog.h>

using ::novaagent::node::v1::ContainerInfo;
using ::novaagent::node::v1::ContainerInfoRequest;

namespace App::Source::Host::Collector::Container {

void ContainerCollector::GetContainerListAsync() const {
    client_->AsyncGetContainerList();
}

void ContainerCollector::OnContainerList(std::vector<ContainerInfo>& container_list) {
    ContainerInfoRequest request;
    for (const auto& container : container_list) {
        auto* container_info = request.add_containers();
        container_info->set_id(container.id);
        container_info->set_name(container.name);
        container_info->set_state(container.state);
        container_info->set_start_time(container.created);
        container_info->set_running_time(container.pid);
        container_info->set_command(container.command);
    }
    SPDLOG_DEBUG("ContainerInfoRequest: {}", request.ShortDebugString());

    {
        std::lock_guard<std::mutex> lock(request_mutex_);
        request_.Swap(&request);
    }
}

void ContainerCollector::GetContainerList(ContainerInfoRequest& request,
                                          const std::unordered_map<int, Process::ProcessSampleInfo>& table) const {
    {
        std::lock_guard<std::mutex> lock(request_mutex_);
        request.CopyFrom(request_);
    }
    auto containers = request.mutable_containers();
    if (containers == nullptr || containers->empty()) {
        return;
    }
    for (const auto& container : *containers) {
        auto it = table.find(container.pid());
        if (it == table.end()) {
            SPDLOG_ERROR("pid {} in container {} not found", container.pid(), container.id());
            continue;
        }
        const auto& process = it->second.process;
        assert(process != nullptr);
        auto* container_info = request.add_containers();
        container_info->set_cpu_limit(0);
        container_info->set_cpu_user_pct(process->cpu_user_pct());
        container_info->set_cpu_system_pct(process->cpu_system_pct());
        container_info->set_cpu_total_pct(process->cpu_total_pct());
        container_info->set_rss_usage(process->memory_usage().rss_pct());
        container_info->set_memory_limit(0);
        container_info->set_memory_cache(process->memory_usage().swap());
        container_info->set_memory_rss(process->memory_usage().resident());
        container_info->set_memory_pct(process->memory_usage().vm_pct());

        container_info->set_rx_bytes(process->iostat_read_bytes());
        container_info->set_rx_bps(process->iostat_read_bytes_rate());
        container_info->set_tx_bytes(process->iostat_write_bytes());
        container_info->set_tx_bps(process->iostat_write_bytes_rate());

        container_info->set_net_rcvd_bps(0);
        container_info->set_net_sent_bps(0);
        container_info->set_pod_ip("");      // todo
        container_info->set_host("");        // todo
        container_info->set_image_name("");  // todo
        container_info->set_health(0);       // todo
        container_info->set_thread_count(0); // todo
        container_info->set_thread_limit(0); // todo
        container_info->set_kube_app("");    // todo
    }
    SPDLOG_DEBUG("ContainerInfoRequest: {}", request.ShortDebugString());
}
} // namespace App::Source::Host::Collector::Container
