#pragma once
#include <component/api.h>  // for Result, Interceptor
#include <memory>           // for unique_ptr

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Log {
namespace Skywalking {
class Processor :public Core::Component::Interceptor {
public:
    Processor() {};

    Core::Component::Result intercept(std::unique_ptr<Core::Component::Batch>& batch) override;

    Core::Component::Result intercept(Core::Component::Batch& batch) override;

    ~Processor() = default;
};
}
}
}
}
}
