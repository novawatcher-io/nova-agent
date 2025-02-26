//
// Created by zhanglei on 2025/2/16.
//
#include "app/include/intercept/opentelemetry/trace/topology/rpc_analysis_listener.h"

#include <unordered_map>
#include <opentelemetry/common/attribute_value.h>
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

#include "app/include/common/opentelemetry/util.h"
#include "app/include/common/const.h"
#include "app/include/common/opentelemetry/const.h"
#include "app/include/intercept/opentelemetry/trace/topology/rpc_traffic_source_builder.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {

using namespace Common::Trace::Skywalking;
using namespace Common::Trace::Topology;
using namespace opentelemetry::proto::trace::v1;

/**
 * Identify the layer of remote service. Such as  ${@link Layer#DATABASE} and ${@link Layer#CACHE}.
 */
App::Common::Trace::LAYER identifyRemoteServiceLayer(int spanLayer) {
    switch (spanLayer) {
        case skywalking::v3::SpanLayer::Unknown:
            return App::Common::Trace::LAYER::UNDEFINED;
        case skywalking::v3::SpanLayer::Database:
            return App::Common::Trace::LAYER::VIRTUAL_DATABASE;
        case skywalking::v3::SpanLayer::RPCFramework:
            return App::Common::Trace::LAYER::GENERAL;
        case skywalking::v3::SpanLayer::Http:
            return App::Common::Trace::LAYER::GENERAL;
        case skywalking::v3::SpanLayer::MQ:
            return App::Common::Trace::LAYER::VIRTUAL_MQ;
        case skywalking::v3::SpanLayer::Cache:
            return App::Common::Trace::LAYER::VIRTUAL_CACHE;
//        case skywalking::v3::SpanLayer::UNRECOGNIZED:
//            return App::Common::Trace::LAYER::UNDEFINED;
        case skywalking::v3::SpanLayer::FAAS:
            return App::Common::Trace::LAYER::FAAS;
        default:
            return App::Common::Trace::LAYER::UNDEFINED;
    }
}

std::string instanceToIp(const std::string& instance) {
    auto pos = instance.find('@');
    if (pos == std::string::npos) {
        return "";
    } else {
        if (pos >= instance.size()) {
            return "";
        }
        return instance.substr(pos + 1);
    }
}

void sourceNameToKubernetesName(const std::string name, const std::string& instance, std::unique_ptr<RPCTrafficSourceBuilder>& sourceBuilder) {
    if (!name.empty()) {
        auto pairs = Common::Kubernetes::NameControl::getInstance()->findServiceName(name);
        if (!pairs.first.empty()) {
            sourceBuilder->sourceServiceName = pairs.first;
            sourceBuilder->sourceNamespace = pairs.second;
            return;
        }
    }

    if (!instance.empty()) {
        auto ip = instanceToIp(instance);
        auto pairs = Common::Kubernetes::NameControl::getInstance()->findServiceName(ip);
        if (!pairs.first.empty()) {
            sourceBuilder->sourceServiceName = pairs.first;
            sourceBuilder->sourceNamespace = pairs.second;
            return;
        }
    }
}

void destNameToKubernetesName(const std::string name, const std::string& instance, std::unique_ptr<RPCTrafficSourceBuilder>& sourceBuilder) {
    if (!name.empty()) {
        auto pairs = Common::Kubernetes::NameControl::getInstance()->findServiceName(name);
        if (!pairs.first.empty()) {
            sourceBuilder->destServiceName = pairs.first;
            sourceBuilder->destNamespace = pairs.second;
            return;
        }
    }

    if (!instance.empty()) {
        auto ip = instanceToIp(instance);
        auto pairs = Common::Kubernetes::NameControl::getInstance()->findServiceName(ip);
        if (!pairs.first.empty()) {
            sourceBuilder->destServiceName = pairs.first;
            sourceBuilder->destNamespace = pairs.second;
            return;
        }
    }
}

void RPCAnalysisListener::parseEntry(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable,
                                     const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& spanAttr) {
    if (!enabled) {
        return;
    }
    auto sourceBuilder = std::make_unique<RPCTrafficSourceBuilder>();
    auto layer = spanAttr.find(AttributeLayer);
    if (layer != spanAttr.end()) {
        sourceBuilder->destLayer = layer->second.int_value();
    }
    auto serviceInstanceNameIter = spanAttr.find(Common::Opentelemetry::AttributeServiceInstanceID);
    if (serviceInstanceNameIter != spanAttr.end()) {
        sourceBuilder->destServiceInstanceName = serviceInstanceNameIter->second.string_value();
    }
    auto serviceIter = spanAttr.find(Common::Opentelemetry::AttributeServiceName);
    if (serviceIter != spanAttr.end()) {
        sourceBuilder->destServiceName = serviceIter->second.string_value();
    }
    // 转换服务名字，如果是k8s环境要进行转换
    destNameToKubernetesName(sourceBuilder->destServiceName, sourceBuilder->destServiceInstanceName, sourceBuilder);
    sourceBuilder->detectPoint = DetectPoint::SERVER;


    if (!span.links().empty()) {
        for (int i = 0; i < span.links_size(); i++) {
            auto link = span.links(i);
            auto attr = App::Common::Opentelemetry::linkAttrToMap(link);
            auto parentEndpointIter = attr.find(AttributeParentEndpoint);
            if (parentEndpointIter == attr.end()) {
                sourceBuilder->sourceEndpointName = UserEndpointName;
            } else {
                sourceBuilder->sourceEndpointName = parentEndpointIter->second.string_value();
            }

            bool isMQ = false;
            if (layer != spanAttr.end()  && layer->second.int_value() == skywalking::v3::SpanLayer::MQ) {
                isMQ = true;
            }
            auto workAddressUsedAtPeerIter = attr.find(AttributeNetworkAddressUsedAtPeer);
            if (isMQ) {
                if (workAddressUsedAtPeerIter != attr.end()) {
                    sourceBuilder->sourceServiceName = workAddressUsedAtPeerIter->second.string_value();
                    sourceBuilder->sourceServiceInstanceName = workAddressUsedAtPeerIter->second.string_value();
                }
                sourceBuilder->sourceLayer = App::Common::Trace::LAYER::MQ;
            } else {
                auto parentServiceIter = attr.find(AttributeParentService);
                if (parentServiceIter != attr.end()) {
                    sourceBuilder->sourceServiceName = parentServiceIter->second.string_value();
                }
                auto parentInstanceIter = attr.find(AttributeParentInstance);
                if (parentInstanceIter != attr.end()) {
                    sourceBuilder->sourceServiceInstanceName = parentInstanceIter->second.string_value();
                }
                sourceBuilder->sourceLayer = App::Common::Trace::LAYER::GENERAL;
            }
            sourceBuilder->destEndpointName = span.name();
            sourceNameToKubernetesName(sourceBuilder->sourceServiceName, sourceBuilder->sourceServiceInstanceName, sourceBuilder);
            callingInTraffic.emplace_back(std::move(sourceBuilder));
        }
    } else if (layer != spanAttr.end() && layer->second.int_value() == skywalking::v3::SpanLayer::MQ) {
        auto peerIter = spanAttr.find(AttributePeer);
        if (peerIter != spanAttr.end()) {
            sourceBuilder->sourceServiceName = peerIter->second.string_value();
            sourceBuilder->sourceServiceInstanceName = peerIter->second.string_value();
        }
        sourceBuilder->destEndpointName = span.name();
        sourceBuilder->sourceLayer = skywalking::v3::SpanLayer::MQ;
        auto componentIdIter = spanAttr.find(AttributeComponentId);
        if (componentIdIter != spanAttr.end()) {
            sourceBuilder->componentId = (int)componentIdIter->second.int_value();
        }
        sourceNameToKubernetesName(sourceBuilder->sourceServiceName, sourceBuilder->sourceServiceInstanceName, sourceBuilder);
        callingInTraffic.emplace_back(std::move(sourceBuilder));
    } else {
        sourceBuilder->sourceServiceName = UserInstanceName;
        sourceBuilder->sourceServiceInstanceName = UserServiceName;
        sourceBuilder->sourceEndpointName = UserEndpointName;
        sourceBuilder->sourceLayer = 0;
        sourceBuilder->destEndpointName = span.name();
        auto componentIdIter = spanAttr.find(AttributeComponentId);
        if (componentIdIter != spanAttr.end()) {
            sourceBuilder->componentId = (int)componentIdIter->second.int_value();
        }
        sourceNameToKubernetesName(sourceBuilder->sourceServiceName, sourceBuilder->sourceServiceInstanceName, sourceBuilder);
        callingInTraffic.emplace_back(std::move(sourceBuilder));
    }
//    parseLogicEndpoints(span, recordable, spanAttr);
}

void RPCAnalysisListener::parseLogicEndpoints(const opentelemetry::proto::trace::v1::Span& span,
const App::Common::Opentelemetry::Recordable* recordable,
const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) {
    auto logicEndpointIter = attr.find(App::Common::Trace::LogicEndpoint);
    if (logicEndpointIter == attr.end()) {
        return;
    }

    rapidjson::Document dom;
    if (dom.Parse(logicEndpointIter->second.string_value().c_str()).HasParseError()) {
        return;
    }
    bool isLocalSpan = false;
    if (span.kind() == Span_SpanKind_SPAN_KIND_INTERNAL) {
        isLocalSpan = true;
    }
    uint64_t latency;
    bool status;
    std::string logicEndpointName;

    if (isLocalSpan && dom.HasMember("logic-span") && dom["logic-span"].GetBool()) {
        logicEndpointName = span.name();
        latency = (span.end_time_unix_nano() - span.start_time_unix_nano());
        status = (span.status().code() == opentelemetry::proto::trace::v1::Status_StatusCode_STATUS_CODE_OK) ? true : false;
    } else if (dom.HasMember("name") && dom.HasMember("latency") && dom.HasMember("status")) {
        logicEndpointName = dom["name"].GetString();
        latency = dom["latency"].GetUint64();
        status = dom["status"].GetBool();
    } else {
        return;
    }

    auto endpointSourceBuilder = std::make_unique<EndpointSourceBuilder>();
    auto serviceIter = attr.find(Common::Opentelemetry::AttributeServiceInstanceID);
    if (serviceIter != attr.end()) {
        endpointSourceBuilder->destServiceName = serviceIter->second.string_value();
    }
    auto serviceInstanceNameIter = attr.find(Common::Opentelemetry::AttributeServiceInstanceID);
    if (serviceInstanceNameIter != attr.end()) {
        endpointSourceBuilder->destServiceInstanceName = serviceInstanceNameIter->second.string_value();
    }
    endpointSourceBuilder->destEndpointName = logicEndpointName;
    endpointSourceBuilder->destLayer = App::Common::Trace::LAYER::GENERAL;
    endpointSourceBuilder->detectPoint = DetectPoint::SERVER;
    endpointSourceBuilder->latency = latency;
    endpointSourceBuilder->type = App::Common::Trace::RequestType::LOGIC;
    endpointSourceBuilder->status = status;

//    sourceBuilder.setTimeBucket(TimeBucket.getMinuteTimeBucket(span.getStartTime()));
    logicEndpointBuilders.emplace_back(std::move(endpointSourceBuilder));
    parseLogicEndpoints(span, recordable, attr);
}

void RPCAnalysisListener::parseExit(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable,
                                    const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) {
    if (!enabled) {
        return;
    }
    auto sourceBuilder = std::make_unique<RPCTrafficSourceBuilder>();
    auto peerIter = attr.find(AttributePeer);
    if (peerIter == attr.end()) {
        return;
    }

    auto serviceIter = attr.find(App::Common::Opentelemetry::AttributeServiceName);
    if (serviceIter != attr.end()) {
        sourceBuilder->sourceServiceName = serviceIter->second.string_value();
    }

    auto serviceInstanceNameIter = attr.find(Common::Opentelemetry::AttributeServiceInstanceID);
    if (serviceInstanceNameIter != attr.end()) {
        sourceBuilder->sourceServiceInstanceName = serviceInstanceNameIter->second.string_value();
    }
    auto componentIdIter = attr.find(AttributeComponentId);
    if (componentIdIter != attr.end()) {
        sourceBuilder->componentId = (int)componentIdIter->second.int_value();
    }

    sourceBuilder->detectPoint = DetectPoint::CLIENT;
    auto layer = attr.find(AttributeLayer);
    if (layer != attr.end()) {
        sourceBuilder->sourceLayer = layer->second.int_value();
    }
    sourceBuilder->destServiceName = peerIter->second.string_value();
    sourceBuilder->destServiceInstanceName = peerIter->second.string_value();
    sourceBuilder->destLayer = identifyRemoteServiceLayer(layer->second.int_value());
    sourceNameToKubernetesName(sourceBuilder->sourceServiceName, sourceBuilder->sourceServiceInstanceName, sourceBuilder);
    destNameToKubernetesName(sourceBuilder->destServiceName, sourceBuilder->destServiceInstanceName, sourceBuilder);
    callingOutTraffic.emplace_back(std::move(sourceBuilder));
}

void RPCAnalysisListener::parseLocal(const opentelemetry::proto::trace::v1::Span &span, const App::Common::Opentelemetry::Recordable *recordable,
                                     const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) {

}

void RPCAnalysisListener::build() {
    for (auto& source : callingInTraffic) {
        auto service = source->toService();
        auto exist = inLruCache->exists(service->id());
        inLruCache->put(service->id(), true);
        if (exist) {
            continue;
        }
        sink_->registerService(*service);
        auto relation = source->toServiceRelation();
        inLruCache->put(relation->id(), true);
        exist = inLruCache->exists(service->id());
        if (exist) {
            continue;
        }
        sink_->registerServiceRelation(*relation);
        continue;
    }

    callingInTraffic.clear();

    for (auto& out : callingOutTraffic) {
        auto relation = out->toServiceRelation();
        auto exist = outLruCache->exists(relation->id());
        if (exist) {
            continue;
        }
        sink_->registerServiceRelation(*relation);
    }
    callingOutTraffic.clear();
}
}