//
// Created by zhanglei on 2025/2/17.
//
#include "app/include/intercept/opentelemetry/trace/topology/endpoint_dep_from_cross_thread_analysis_listener.h"

#include "app/include/common/const.h"
#include "app/include/common/opentelemetry/util.h"
#include "app/include/common/opentelemetry/const.h"
#include "app/include/intercept/opentelemetry/trace/topology/common.h"
#include "app/include/intercept/opentelemetry/trace/topology/rpc_traffic_source_builder.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {

using namespace opentelemetry::proto::trace::v1;
using namespace Common::Trace::Skywalking;
using namespace Common::Trace::Topology;

void EndpointDepFromCrossThreadAnalysisListener::parseEntry(const opentelemetry::proto::trace::v1::Span &span,
const App::Common::Opentelemetry::Recordable *recordable,
const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) {

}

void EndpointDepFromCrossThreadAnalysisListener::parseExit(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable,
const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) {
    parseRefForEndpointDependency(span, recordable, attr);
}

void EndpointDepFromCrossThreadAnalysisListener::parseLocal(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable,
const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) {
    parseRefForEndpointDependency(span, recordable, attr);
}

void EndpointDepFromCrossThreadAnalysisListener::build() {

}

void EndpointDepFromCrossThreadAnalysisListener::parseRefForEndpointDependency(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable,
const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) {
    if (!span.links().empty()) {
        for (int i = 0; i < span.links_size(); i++) {
            auto link = span.links(i);
            auto linkAttr = App::Common::Opentelemetry::linkAttrToMap(link);
            auto sourceBuilder = std::make_unique<RPCTrafficSourceBuilder>();
            auto parentEndpointIter = attr.find(AttributeParentEndpoint);
            if (parentEndpointIter == attr.end()) {
                sourceBuilder->sourceEndpointName = UserEndpointName;
            } else {
                sourceBuilder->sourceEndpointName = parentEndpointIter->second.string_value();
            }
            auto peerIter = linkAttr.find(AttributeNetworkAddressUsedAtPeer);
            auto layer = attr.find(AttributeLayer);
            auto parentServiceIter = linkAttr.find(AttributeParentService);
            auto serviceInstanceIter = attr.find(App::Common::Opentelemetry::AttributeServiceInstanceID);
            auto serviceIter = attr.find(App::Common::Opentelemetry::AttributeServiceName);
            auto parentServiceInstanceIter = linkAttr.find(AttributeParentInstance);
            auto componentIdIter = attr.find(AttributeComponentId);
            bool isMq = false;
            if (layer != attr.end() && layer->second.int_value() == skywalking::v3::SpanLayer::MQ) {
                isMq = true;
            }
            if (isMq) {
                if (peerIter != linkAttr.end()) {
                    sourceBuilder->sourceServiceName = peerIter->second.string_value();
                    sourceBuilder->sourceServiceInstanceName = peerIter->second.string_value();
                }
                if (parentServiceIter != attr.end()) {
                    sourceBuilder->sourceEndpointOwnerServiceName = parentServiceIter->second.string_value();
                }
                sourceBuilder->sourceLayer = App::Common::Trace::LAYER::MQ;
                sourceBuilder->sourceEndpointOwnerServiceLayer = App::Common::Trace::LAYER::GENERAL;
            } else {
                if (parentServiceIter != attr.end()) {
                    sourceBuilder->sourceServiceName = parentServiceIter->second.string_value();
                }

                if (parentServiceInstanceIter != attr.end()) {
                    sourceBuilder->sourceServiceInstanceName = parentServiceInstanceIter->second.string_value();
                }
                sourceBuilder->sourceLayer = App::Common::Trace::LAYER::GENERAL;
            }
            sourceBuilder->destEndpointName = span.name();
            if (serviceInstanceIter != attr.end()) {
                sourceBuilder->destServiceInstanceName = serviceInstanceIter->second.string_value();
            }
            if (serviceIter != attr.end()) {
                sourceBuilder->destServiceName = serviceIter->second.string_value();
            }
            sourceBuilder->detectPoint = DetectPoint::SERVER;
            if (layer != attr.end()) {
                sourceBuilder->destLayer = identifyServiceLayer(layer->second.int_value());
            }
            if (componentIdIter != attr.end()) {
                sourceBuilder->componentId = componentIdIter->second.int_value();
            }
            auto dep = std::make_unique<EndpointDependencyBuilder>();
            depBuilders.emplace_back(std::move(dep));
        }
    }
}
}