#pragma once
#include "app/include/source/host/collector/gpu/gpu_reader.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include <string>
#include <vector>

namespace App::Source::Host::Collector::GPU {
class GPUCollector {
public:
    explicit GPUCollector(const std::string& company_uuid);

    void Start() const;
    void Stop() const;

private:
    std::string company_uuid_;
    GPUReader gpu_reader_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter_;
    std::vector<std::unique_ptr<Oltp::MetricCollector>> gpu_metrics_;
};
} // namespace App::Source::Host::Collector::GPU
