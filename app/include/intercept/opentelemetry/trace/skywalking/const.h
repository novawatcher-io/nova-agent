//
// Created by root on 2024/4/1.
//

#pragma once

#include <string>
#include <unordered_map>
#include "opentelemetry/trace/semantic_conventions.h"

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Trace {
namespace Skywalking {

static const std::unordered_map<std::string, std::string> otSpanTagsMapping = std::unordered_map<std::string, std::string> {
        {"url", opentelemetry::trace::SemanticConventions::kUrlFull},
        {"status_code", opentelemetry::trace::SemanticConventions::kHttpResponseStatusCode},
        {"db.type", opentelemetry::trace::SemanticConventions::kDbSystem},
        {"db.instance", opentelemetry::trace::SemanticConventions::kDbName},
        {"mq.broker",   opentelemetry::trace::SemanticConventions::kServerAddress}
};
}
}
}
}
}