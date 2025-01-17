#include "app/include/intercept/opentelemetry/log/skywalking/processor.h"
#include <string>           // for basic_string
#include "component/api.h"  // for Batch (ptr only), Result

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Log {
namespace Skywalking {
Core::Component::Result Processor::intercept(Core::Component::Batch &) {
    return {};
}

Core::Component::Result Processor::intercept(std::unique_ptr<Core::Component::Batch> &) {
    return {};
}
}
}
}
}
}
