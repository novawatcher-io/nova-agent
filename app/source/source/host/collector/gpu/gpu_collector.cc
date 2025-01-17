#include "app/include/source/host/collector/gpu/gpu_collector.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include "source/host/collector/gpu/gpu_reader.h"
#include <memory>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/provider.h>
#include <spdlog/spdlog.h>
#include <stdint.h>

using App::Source::Host::Collector::Oltp::MetricCollector;
using App::Source::Host::Collector::Oltp::MetricData;
using App::Source::Host::Collector::Oltp::MultiValue;
using App::Source::Host::Collector::Oltp::SingleValue;

namespace App::Source::Host::Collector::GPU {

GPUCollector::GPUCollector(const std::string& company_uuid) : company_uuid_(company_uuid) {
    if (!gpu_reader_.IsGPUAvailable()) {
        SPDLOG_INFO("GPU is not available");
        return;
    }
    auto provider = opentelemetry::metrics::Provider::GetMeterProvider();
    meter_ = provider->GetMeter("gpu", "1.2.0");

    gpu_metrics_.emplace_back(MetricCollector::Create<int64_t>(
        meter_, company_uuid_, "gpu_usage", [&](MultiValue& values) { gpu_reader_.GetGPUUsage(values); }));

    gpu_metrics_.emplace_back(MetricCollector::Create<int64_t>(
        meter_, company_uuid_, "gpu_memory_usage", [&](MultiValue& values) { gpu_reader_.GetGPUMemoryUsage(values); }));

    gpu_metrics_.emplace_back(MetricCollector::Create<int64_t>(
        meter_, company_uuid_, "gpu_memory_used", [&](MultiValue& values) { gpu_reader_.GetGPUMemoryUsed(values); }));

    gpu_metrics_.emplace_back(MetricCollector::Create<int64_t>(
        meter_, company_uuid_, "gpu_memory_free", [&](MultiValue& values) { gpu_reader_.GetGPUMemoryFree(values); }));

    gpu_metrics_.emplace_back(MetricCollector::Create<int64_t>(
        meter_, company_uuid_, "gpu_temperature", [&](MultiValue& values) { gpu_reader_.GetGPUTemperature(values); }));

    gpu_metrics_.emplace_back(MetricCollector::Create<int64_t>(
        meter_, company_uuid_, "gpu_power_draw", [&](MultiValue& values) { gpu_reader_.GetGPUPowerDraw(values); }));

    gpu_metrics_.emplace_back(MetricCollector::Create<int64_t>(
        meter_, company_uuid_, "gpu_core_clock", [&](MultiValue& values) { gpu_reader_.GetGPUCoreClock(values); }));

    gpu_metrics_.emplace_back(
        MetricCollector::Create<int64_t>(meter_, company_uuid_, "gpu_process_memory_usage",
                                         [&](MultiValue& values) { gpu_reader_.GetGPUProcessMemoryUsage(values); }));

    gpu_metrics_.emplace_back(
        MetricCollector::Create<int64_t>(meter_, company_uuid_, "gpu_pcie_rx_bandwidth",
                                         [&](MultiValue& values) { gpu_reader_.GetGPUPCIERXBandwidth(values); }));

    gpu_metrics_.emplace_back(
        MetricCollector::Create<int64_t>(meter_, company_uuid_, "gpu_pcie_tx_bandwidth",
                                         [&](MultiValue& values) { gpu_reader_.GetGPUPCIETXBandwidth(values); }));

    gpu_metrics_.emplace_back(MetricCollector::Create<int64_t>(
        meter_, company_uuid_, "gpu_ecc_errors", [&](MultiValue& values) { gpu_reader_.GetGPUEccErrors(values); }));
}

void GPUCollector::Start() const {
    if (!gpu_reader_.IsGPUAvailable()) {
        return;
    }
    for (auto& metric : gpu_metrics_) {
        metric->Start();
    }
}

void GPUCollector::Stop() const {
    if (!gpu_reader_.IsGPUAvailable()) {
        return;
    }
    for (auto& metric : gpu_metrics_) {
        metric->Stop();
    }
}
} // namespace App::Source::Host::Collector::GPU
