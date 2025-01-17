#pragma once
#include <memory>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <string>

namespace App::Prometheus {
class PrometheusExposer {
public:
    explicit PrometheusExposer(const std::string& addr);

    prometheus::Counter& AddCounter(const std::string& name, const std::string& help,
                                    std::initializer_list<prometheus::Labels::value_type> labels);

private:
    prometheus::Exposer exposer_;
    std::shared_ptr<prometheus::Registry> registry_;
};

} // namespace App::Prometheus
