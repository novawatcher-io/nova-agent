//
// Created by root on 2024/4/12.
//

#pragma once

#include <component/api.h>

#include "sky_metric_resource.h"
#include "app/include/common/opentelemetry/otlp_metric_resource.h"
namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Metric {
namespace Skywalking {
class SkyMetricData :public Common::Opentelemetry::OtlpMetricResource {
public:
    explicit SkyMetricData(std::unique_ptr<opentelemetry::sdk::resource::Resource> resourcePtr) :resource(std::move(resourcePtr)){
        resource_ = resource.get();
    }


    ~SkyMetricData() = default;
private:
    std::unique_ptr<opentelemetry::sdk::resource::Resource> resource;
};
}
}
}
}
}
