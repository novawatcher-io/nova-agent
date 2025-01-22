#include "app/include/prometheus/prometheus_exposer.h"
#include <initializer_list>
#include <memory>
#include <prometheus/counter.h>
#include <prometheus/labels.h>
#include <string>
#include <http/http_response.h>
#include <http/http_request.h>

namespace App::Prometheus {

PrometheusExposer::PrometheusExposer() {
    registry_ = std::make_shared<prometheus::Registry>();
}

prometheus::Counter& PrometheusExposer::AddCounter(const std::string& name, const std::string& help,
                                                   std::initializer_list<prometheus::Labels::value_type> labels) {
    auto& counter_family = prometheus::BuildCounter().Name(name).Help(help).Register(*registry_);
    auto& counter = counter_family.Add(labels);
    return counter;
}

void PrometheusExposer::handle(Core::Http::HttpRequest &request, Core::Http::HttpResponse &response) {
    response.response(200, serializer.Serialize(registry_->Collect()));
}
} // namespace App::Prometheus
