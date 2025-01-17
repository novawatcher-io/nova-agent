//
// Created by root on 2024/4/4.
//

#pragma once

#include <opentelemetry/sdk/resource/resource.h>

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Trace {
namespace Skywalking {
class SkyResource :public opentelemetry::sdk::resource::Resource {
public:
    SkyResource(const opentelemetry::sdk::resource::ResourceAttributes& attributes, const std::string& schema_url)
        : opentelemetry::sdk::resource::Resource(attributes, schema_url) {
    }

    ~SkyResource() = default;

};
}
}
}
}
}
