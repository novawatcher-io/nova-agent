//
// Created by zhanglei on 2025/2/16.
//
#include "app/include/intercept/opentelemetry/trace/topology/rpc_analysis_listener.h"

#include <unordered_map>
#include <opentelemetry/common/attribute_value.h>
#include <skywalking-data-collect-protocol/language-agent/Tracing.grpc.pb.h>

#include "app/include/common/const.h"
#include "app/include/common/opentelemetry/const.h"
#include "app/include/intercept/opentelemetry/trace/topology/rpc_traffic_source_builder.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
using namespace opentelemetry::proto::trace::v1;
using namespace Common::Trace::Skywalking;
using namespace Common::Trace::Topology;

// link attr to map
std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue> linkAttrToMap(Span_Link& link) {
    std::unordered_map <std::string, ::opentelemetry::proto::common::v1::AnyValue> attrs;
    for (auto iter = link.attributes().begin(); iter != link.attributes().end(); iter++) {
        attrs[iter->key()] = iter->value();
    }
    return attrs;
}

// span attr to map
std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue> attrToMap(const opentelemetry::proto::trace::v1::Span &span) {
    std::unordered_map <std::string, ::opentelemetry::proto::common::v1::AnyValue> attrs;
    for (auto iter = span.attributes().begin(); iter != span.attributes().end(); iter++) {
        attrs[iter->key()] = iter->value();
    }
    return attrs;
}

void RPCAnalysisListener::parseEntry(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable) {
    auto spanAttr = attrToMap(span);
    auto layer = spanAttr.find(AttributeLayer);
    if (!span.links().empty()) {
        for (int i = 0; i < span.links_size(); i++) {
            auto link = span.links(i);
            auto sourceBuilder = std::make_unique<RPCTrafficSourceBuilder>();
            auto attr = linkAttrToMap(link);
            auto parentEndpointIter = attr.find(AttributeParentEndpoint);
            if (parentEndpointIter == attr.end()) {
                sourceBuilder->sourceEndpointName = UserEndpointName;
            } else {
                sourceBuilder->sourceEndpointName = parentEndpointIter->second.string_value();
            }
            sourceBuilder->destEndpointName = span.name();

            auto serviceInstanceNameIter = spanAttr.find(Common::Opentelemetry::AttributeServiceInstanceID);
            if (serviceInstanceNameIter != spanAttr.end()) {
                sourceBuilder->destServiceInstanceName = serviceInstanceNameIter->second.string_value();
            }
            auto serviceIter = spanAttr.find(Common::Opentelemetry::AttributeServiceInstanceID);
            if (serviceIter != spanAttr.end()) {
                sourceBuilder->destServiceName = serviceIter->second.string_value();
            }
            if (layer != spanAttr.end()) {
                sourceBuilder->destLayer = layer->second.int_value();
            }
            sourceBuilder->detectPoint = DetectPoint::SERVER;
            callingInTraffic.emplace_back(std::move(sourceBuilder));
        }
    } else if (layer != spanAttr.end() && layer->second.int_value() == skywalking::v3::SpanLayer::MQ) {

    } else {

    }
}

void RPCAnalysisListener::parseExit(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable) {

}

void RPCAnalysisListener::parseLocal(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable) {

}
}