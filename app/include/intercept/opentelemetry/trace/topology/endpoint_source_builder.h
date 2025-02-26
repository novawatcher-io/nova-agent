//
// Created by zhanglei on 2025/2/16.
//

#pragma once

#include <string>
#include <list>
#include <map>

#include "app/include/common/const.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
class EndpointSourceBuilder {
public:
    long timeBucket;

    std::string destServiceName;

    int destLayer;

    std::string destServiceInstanceName;

    std::string destEndpointName;

    std::string destNamespace;

    int latency;

    bool status;

    int httpResponseStatusCode;

    std::string rpcStatusCode;

    int type;

    Common::Trace::Topology::DetectPoint detectPoint;

    std::list<std::string> tags;

    std::map<std::string, std::string> originalTags;
};
}