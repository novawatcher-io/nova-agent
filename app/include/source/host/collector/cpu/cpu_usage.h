#pragma once
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include "app/include/source/host/collector/process/proc_reader.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include <memory>
#include <opentelemetry/metrics/meter.h>
#include <string>
#include <vector>

namespace App::Source::Host::Collector::Cpu {
namespace metrics_api = opentelemetry::metrics;

class CpuUsage {
public:
    explicit CpuUsage(const std::string& company_uuid, Process::ProcReader* reader);

    void Start();

    void Stop();

private:
    void GetCpuUsage(Oltp::SingleValue& value);

    Process::StatInfo last_;
    Process::ProcReader* proc_reader_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter_;
    std::vector<std::unique_ptr<Oltp::MetricCollector>> metrics_;
};
} // namespace App::Source::Host::Collector::Cpu
