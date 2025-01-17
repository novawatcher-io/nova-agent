#include "app/include/source/host/collector/cpu/cpu_usage.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include "app/include/source/host/collector/process/proc_reader.h"
#include "opentelemetry/metrics/meter_provider.h"
#include <opentelemetry/metrics/provider.h>
#include <spdlog/spdlog.h>

using App::Source::Host::Collector::Oltp::MetricCollector;
using App::Source::Host::Collector::Oltp::MultiValue;
using App::Source::Host::Collector::Oltp::SingleValue;

namespace App::Source::Host::Collector::Cpu {

CpuUsage::CpuUsage(const std::string& company_uuid, Process::ProcReader* reader) : proc_reader_(reader) {
    auto provider = metrics_api::Provider::GetMeterProvider();
    meter_ = provider->GetMeter("system_cpu", "1.2.0");
    metrics_.emplace_back(MetricCollector::Create<double>(meter_, company_uuid, "system_cpu_usage",
                                                          [&](SingleValue& value) { GetCpuUsage(value); }));
}

void CpuUsage::Start() {
    for (auto& metric : metrics_) {
        metric->Start();
    }
}

void CpuUsage::Stop() {
    for (auto& metric : metrics_) {
        metric->Stop();
    }
}

void CpuUsage::GetCpuUsage(SingleValue& value) {
    App::Source::Host::Collector::Process::StatInfo curr;
    proc_reader_->GetProcStat(curr);
    double cpu_usage = 0;
    if (last_.cpu_time.CPUTotalTime() != 0) {
        const double total_delta = curr.cpu_time.CPUTotalTime() - last_.cpu_time.CPUTotalTime();
        const double idle_delta = curr.cpu_time.CPUIdleTime() - last_.cpu_time.CPUIdleTime();
        if (total_delta > 0) {
            cpu_usage = (total_delta - idle_delta) / total_delta;
        }
    }
    last_ = curr;
    value = cpu_usage;
    SPDLOG_INFO("CPU 使用率: {}", cpu_usage);
}
} // namespace App::Source::Host::Collector::Cpu
