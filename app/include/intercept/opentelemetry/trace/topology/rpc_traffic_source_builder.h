//
// Created by zhanglei on 2025/2/16.
//
#pragma once

#include <string>

#include "endpoint_source_builder.h"
#include "common/xxhash64.h"
#include <trace/v1/topology.grpc.pb.h>

namespace App::Intercept::Opentelemetry::Trace::Topology {
class RPCTrafficSourceBuilder :public EndpointSourceBuilder {
public:
    std::string sourceServiceName;

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

    std::unique_ptr<novaagent::trace::v1::Service> toService() {
        std::unique_ptr<novaagent::trace::v1::Service> service = std::make_unique<novaagent::trace::v1::Service>();
        Core::Common::XXHash64 hashUtil(0);
        std::string key = destServiceName + std::to_string(destLayer);
        hashUtil.add(key.data(), key.length());
        service->set_id(hashUtil.hash());
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

    std::unique_ptr<novaagent::trace::v1::ServiceRelation> toServiceRelation() {
        std::unique_ptr<novaagent::trace::v1::ServiceRelation> relation = std::make_unique<novaagent::trace::v1::ServiceRelation>();
        if (sourceServiceName.empty()) {
            relation->set_sourceservicename(sourceServiceName);
        }
        uint64_t sourceServiceId = 0;
        relation->set_sourcelayer(sourceLayer);
        relation->set_sourceservicename(sourceServiceName);
//        relation->set_sourceserviceinstancename(sourceServiceInstanceName);
        relation->set_destservicename(destServiceName);
//        relation->set_destserviceinstancename(destServiceInstanceName);
        relation->set_destlayer(destLayer);
        if (!sourceServiceName.empty()) {
            sourceServiceId = makeHashId(sourceServiceName + std::to_string(sourceLayer));
        }
        relation->set_sourceserviceid(sourceServiceId);
        uint64_t destServiceId = 0;
        if (!destServiceName.empty()) {
            destServiceId = makeHashId(destServiceName + std::to_string(destLayer));
            relation->set_destserviceid(destServiceId);
        } else {
            relation->set_destserviceid(0);
        }
        relation->set_id(makeHashId(std::to_string(sourceServiceId) + std::to_string(destServiceId)));

        return relation;
    }
};
}
