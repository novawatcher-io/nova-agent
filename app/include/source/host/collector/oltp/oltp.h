#pragma once
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include <memory>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <string>
#include <vector>

namespace App {
namespace Source {
namespace Host {
namespace Collector {
namespace Process {
class ProcReader;
}
} // namespace Collector
} // namespace Host
} // namespace Source
} // namespace App

namespace App::Source::Host::Collector::Oltp {

class OltpCollector {
public:
    explicit OltpCollector(Process::ProcReader* reader);
    void Start();
    void Stop();

private:
    std::string company_uuid_;
    Process::ProcReader* proc_reader_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter_;
    std::vector<std::unique_ptr<MetricCollector>> misc_metrics_;
};
} // namespace App::Source::Host::Collector::Oltp
