#pragma once
#include <memory>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <string>
#include <http/http_action.h>
#include <prometheus/text_serializer.h>

namespace App::Prometheus {
class PrometheusExposer {
public:
  PrometheusExposer();

    prometheus::Counter& AddCounter(const std::string& name, const std::string& help,
                                    std::initializer_list<prometheus::Labels::value_type> labels);

    void handle(Core::Http::HttpRequest &request, Core::Http::HttpResponse &response);

private:
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::TextSerializer serializer;
};

} // namespace App::Prometheus
