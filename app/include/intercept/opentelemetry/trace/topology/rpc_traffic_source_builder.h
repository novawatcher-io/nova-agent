//
// Created by zhanglei on 2025/2/16.
//
#pragma once

#include <string>

#include "endpoint_source_builder.h"

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
};
}
