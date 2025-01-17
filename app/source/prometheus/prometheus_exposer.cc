#include "app/include/prometheus/prometheus_exposer.h"
#include <initializer_list>
#include <memory>
#include <prometheus/counter.h>
#include <prometheus/labels.h>
#include <string>

namespace App::Prometheus {

PrometheusExposer::PrometheusExposer(const std::string& addr) : exposer_(addr) {
    registry_ = std::make_shared<prometheus::Registry>();
    exposer_.RegisterCollectable(registry_);
}

prometheus::Counter& PrometheusExposer::AddCounter(const std::string& name, const std::string& help,
                                                   std::initializer_list<prometheus::Labels::value_type> labels) {
    auto& counter_family = prometheus::BuildCounter().Name(name).Help(help).Register(*registry_);
    auto& counter = counter_family.Add(labels);
    return counter;
}
} // namespace App::Prometheus
