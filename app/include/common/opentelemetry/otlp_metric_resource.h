//
// Created by root on 2024/4/20.
//

#pragma once
#include <opentelemetry/sdk/metrics/export/metric_producer.h>
namespace App {
namespace Common {
namespace Opentelemetry {
class OtlpMetricResource :public opentelemetry::sdk::metrics::ResourceMetrics, public Core::Component::Data {
public:
    OtlpMetricResource() = default;
    virtual ~OtlpMetricResource() {
        return;
    }
    void* data() override {
        return this;
    }
};
}
}
}