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
static std::string AttributeSkywalkingSegmentID = "sw8.segment_id";
static std::string AttributeRefType = "refType";
static std::string  AttributeParentService             = "parent.service";
static std::string  AttributeParentInstance            = "parent.service.instance";
static std::string  AttributeParentEndpoint            = "parent.endpoint";
static std::string  AttributeSkywalkingSpanID          = "sw8.span_id";
static std::string AttributeSkywalkingTraceID         = "sw8.trace_id";
static std::string AttributeSkywalkingParentSpanID    = "sw8.parent_span_id";
static std::string AttributeSkywalkingParentSegmentID = "sw8.parent_segment_id";
static std::string AttributeNetworkAddressUsedAtPeer  = "network.AddressUsedAtPeer";
static std::string AttributePeer = "sw8.peer";
static std::string AttributeComponentId = "sw8.componentId";

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