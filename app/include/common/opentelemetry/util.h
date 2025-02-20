//
// Created by zhanglei on 2025/2/17.
//

#pragma once

#include <unordered_map>
#include <string>
#include <skywalking-data-collect-protocol/language-agent/Tracing.grpc.pb.h>

namespace App {
namespace Common {
namespace Opentelemetry {

using namespace opentelemetry::proto::trace::v1;

// link attr to map
static std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue> linkAttrToMap(Span_Link& link) {
    std::unordered_map <std::string, ::opentelemetry::proto::common::v1::AnyValue> attrs;
    for (auto iter = link.attributes().begin(); iter != link.attributes().end(); iter++) {
        attrs[iter->key()] = iter->value();
    }
    return attrs;
}

}
}
}


