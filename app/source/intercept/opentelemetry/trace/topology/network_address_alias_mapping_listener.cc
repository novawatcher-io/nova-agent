//
// Created by zhanglei on 2025/2/18.
//
#include "app/include/intercept/opentelemetry/trace/topology/network_address_alias_mapping_listener.h"

#include "app/include/common/const.h"
#include "app/include/common/opentelemetry/util.h"
#include "app/include/common/opentelemetry/const.h"
#include "app/include/intercept/opentelemetry/trace/topology/common.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {

using namespace opentelemetry::proto::trace::v1;
using namespace skywalking::v3;
using namespace Common::Trace::Skywalking;
using namespace Common::Trace::Topology;

void NetworkAddressAliasMappingListener::parseEntry(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable, const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue> &attr) {
    if (!enabled) {
        return;
    }
    if (span.links().empty()) {
        return;
    }
    return;
    auto layer = attr.find(AttributeLayer);
    if (layer != attr.end() && layer->second.int_value() == skywalking::v3::SpanLayer::MQ) {
        for (int i = 0; i < span.links_size(); i++) {
            auto link = span.links(i);
            auto linkAttr = App::Common::Opentelemetry::linkAttrToMap(link);
            auto refIter =  linkAttr.find(AttributeRefType);
            if (refIter != linkAttr.end() && refIter->second.int_value() == RefType::CrossProcess) {

            }
        }
    }
}

void NetworkAddressAliasMappingListener::parseExit(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable, const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue> &attr) {

}

void NetworkAddressAliasMappingListener::parseLocal(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable, const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue> &attr) {

}

void NetworkAddressAliasMappingListener::build() {

}
}