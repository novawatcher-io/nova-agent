//
// Created by root on 2024/4/16.
//
#pragma once

#include <opentelemetry/sdk/resource/resource.h>

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Metric {
namespace Skywalking {
class SkyMetricResource :public opentelemetry::sdk::resource::Resource {
public:
    SkyMetricResource(const opentelemetry::sdk::resource::ResourceAttributes &attributes, const std::string &schema_url)
            : opentelemetry::sdk::resource::Resource(attributes, schema_url) {}

    ~SkyMetricResource() = default;
};
}
}
}
}
}
