//
// Created by root on 2024/4/12.
//
#pragma once

#include <skywalking-data-collect-protocol/language-agent/JVMMetric.grpc.pb.h>
#include "app/include/common/opentelemetry/otlp_event_data.h"
#include "sky_metric_data.h"
#include "sky_metric_resource.h"

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Metric {
namespace Skywalking {
class Processor :public Core::Component::Interceptor {
public:
    Processor() {};

    ~Processor() = default;

    Core::Component::Result intercept(std::unique_ptr<Core::Component::Batch>& batch) override;

    Core::Component::Result intercept(Core::Component::Batch& batch) override;

private:
    std::unique_ptr<App::Common::Opentelemetry::OtlpEventData> process(std::unique_ptr<Core::Component::EventData> &event);
};
}
}
}
}
}
