#include "app/include/source/host/collector/oltp/oltp.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include "node/v1/info.pb.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "source/host/collector/process/proc_reader.h"
#include <cstdint>
#include <opentelemetry/metrics/provider.h>

using App::Source::Host::Collector::Oltp::MetricCollector;
using App::Source::Host::Collector::Oltp::SingleValue;
using App::Source::Host::Collector::Process::NetDevList;

namespace App::Source::Host::Collector::Oltp {
OltpCollector::OltpCollector(Process::ProcReader* reader) : proc_reader_(reader) {
    auto provider = opentelemetry::metrics::Provider::GetMeterProvider();
    meter_ = provider->GetMeter("system", "1.2.0");

    misc_metrics_.emplace_back(
        MetricCollector::Create<double>(meter_, company_uuid_, "system_mem_pct_total_used", [&](SingleValue& value) {
            deepagent::node::v1::VirtualMemoryInfo memory_info;
            proc_reader_->GetMemoryInfo(&memory_info);
            if (memory_info.total() > 0) {
                value = static_cast<double>(memory_info.used()) / memory_info.total();
            }
        }));

    misc_metrics_.emplace_back(
        MetricCollector::Create<double>(meter_, company_uuid_, "system_mem_pct_usable", [&](SingleValue& values) {
            deepagent::node::v1::VirtualMemoryInfo memory_info;
            proc_reader_->GetMemoryInfo(&memory_info);
            if (memory_info.total() > 0) {
                values = static_cast<double>(memory_info.free()) / memory_info.total();
            }
        }));

    misc_metrics_.emplace_back(
        MetricCollector::Create<int64_t>(meter_, company_uuid_, "sum_system_net_bytes_rcvd", [&](SingleValue& values) {
            NetDevList dev_list;
            proc_reader_->GetNetDev(dev_list);
            values = static_cast<int64_t>(dev_list.rx_bytes_total());
        }));

    misc_metrics_.emplace_back(
        MetricCollector::Create<int64_t>(meter_, company_uuid_, "sum_system_net_bytes_sent", [&](SingleValue& values) {
            NetDevList dev_list;
            proc_reader_->GetNetDev(dev_list);
            values = static_cast<int64_t>(dev_list.tx_bytes_total());
        }));
}

void OltpCollector::Start() {
    for (auto& metric : misc_metrics_) {
        metric->Start();
    }
}

void OltpCollector::Stop() {
    for (auto& metric : misc_metrics_) {
        metric->Stop();
    }
}
} // namespace App::Source::Host::Collector::Oltp
