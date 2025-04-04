//
// Created by zhanglei on 2025/2/16.
//
#pragma once

#include <string>

#include "endpoint_source_builder.h"
#include "common/xxhash64.h"
#include "app/include/common/kubernetes/name_control.h"
#include <trace/v1/topology.grpc.pb.h>

namespace App::Intercept::Opentelemetry::Trace::Topology {
class RPCTrafficSourceBuilder :public EndpointSourceBuilder {
public:
    std::string sourceServiceName;

    std::string sourceNamespace;

    int sourceLayer;

    std::string sourceServiceInstanceName;

    /**
     * Same as {@link #sourceEndpointOwnerServiceName}
     * Source endpoint could be not owned by {@link #sourceServiceName}, such as in the MQ or un-instrumented proxy
     * cases. This service always comes from the span.ref, so it is always a general service.
     *
     * @since 9.0.0
     */
    int sourceEndpointOwnerServiceLayer;

    /**
     * Source endpoint could be not owned by {@link #sourceServiceName}, such as in the MQ or un-instrumented proxy
     * cases. This service always comes from the span.ref, so it is always a general service.
     */
    std::string sourceEndpointOwnerServiceName;

    std::string sourceEndpointName;

    int componentId;

    uint64_t sourceId = 0;

    uint64_t destId = 0;

    uint64_t relation_id = 0;

    bool hasError = false;

    std::string cluster;

    uint64_t latency = 0;

    std::unique_ptr<novaagent::trace::v1::Service> toService() {
        std::unique_ptr<novaagent::trace::v1::Service> service = std::make_unique<novaagent::trace::v1::Service>();
        service->set_id(getDestId());
        service->set_name(destServiceName);
//        service->set_serviceinstancename(destServiceInstanceName);
        service->set_type(std::to_string(type));
        return service;
    }

    static uint64_t makeHashId(std::string key) {
        Core::Common::XXHash64 hashUtil(0);
        hashUtil.add(key.data(), key.length());
        return hashUtil.hash();
    }

    uint64_t getSourceId() {
        if (sourceId > 0) {
            return sourceId;
        }
        sourceId = makeHashId(sourceServiceName + sourceNamespace);
        return sourceId;
    }

    uint64_t getDestId() {
        if (destId > 0) {
            return destId;
        }
        destId = makeHashId(destServiceName + destNamespace);
        return destId;
    }

    std::unique_ptr<novaagent::trace::v1::ServiceRelation> toServiceRelation() {
        std::unique_ptr<novaagent::trace::v1::ServiceRelation> relation = std::make_unique<novaagent::trace::v1::ServiceRelation>();
        if (sourceServiceName.empty()) {
            relation->set_sourceservicename(sourceServiceName);
        }
        uint64_t sourceServiceId = 0;
        relation->set_sourcelayer(sourceLayer);
        relation->set_sourceservicename(sourceServiceName);
        relation->set_componentid(std::to_string(componentId));
//        relation->set_sourceserviceinstancename(sourceServiceInstanceName);
        relation->set_destservicename(destServiceName);
//        relation->set_destserviceinstancename(destServiceInstanceName);
        relation->set_destlayer(destLayer);
        if (!sourceServiceName.empty()) {
            sourceServiceId = getSourceId();
        }
        relation->set_sourceserviceid(sourceServiceId);
        uint64_t destServiceId = 0;
        if (!destServiceName.empty()) {
            destServiceId = getDestId();
            relation->set_destserviceid(destServiceId);
        } else {
            relation->set_destserviceid(0);
        }
        relation->set_id(makeHashId(std::to_string(sourceServiceId) + std::to_string(destServiceId)));
        relation_id = relation->id();
        return relation;
    }
};
}
