//
// Created by zhanglei on 2025/2/25.
//

#include "app/include/common/kubernetes/service.h"

#include <fmt/format.h>

namespace App::Common::Kubernetes {
bool Service::parseFromString(rapidjson::Document &document) {
    if (!document.HasMember("object")) {
        return false;
    }

    if (!document.FindMember("object")->value.IsObject()) {
        return false;
    }

    if (!document.FindMember("object")->value.HasMember("metadata")) {
        return false;
    }

    if (!document.FindMember("object")->value.FindMember("metadata")->value.IsObject()) {
        return false;
    }

    if (!document.FindMember("object")->value.FindMember("metadata")->value.HasMember("name")) {
        return false;
    }

    auto metadata = document.FindMember("object")->value.FindMember("metadata");

    name = metadata->value.FindMember("name")->value.GetString();
    if (!metadata->value.HasMember("namespace")) {
        return false;
    }
    namespaceValue = metadata->value.FindMember("namespace")->value.GetString();

    if (!document.HasMember("object")) {
        return false;
    }

    auto objectIter = document.FindMember("object");
    if (!objectIter->value.HasMember("spec")) {
        return false;
    }

    auto spec = objectIter->value.FindMember("spec");
    if (!spec->value.IsObject()) {
        return false;
    }

    if (!spec->value.HasMember("ports")) {
        return false;
    }

    auto portsIter = spec->value.FindMember("ports");
    if (!portsIter->value.IsArray()) {
        return false;
    }

    ports.resize(portsIter->value.Size());
    auto postsArray = portsIter->value.GetArray();
    int index = 0;
    for (auto &port : postsArray) {
        if (!port.HasMember("port")) {
            return false;
        }
        auto portIter = port.FindMember("port");
        if (!port.HasMember("protocol")) {
            return false;
        }
        auto protocolIter = port.FindMember("protocol");
        if (!port.HasMember("targetPort")) {
            return false;
        }
        auto targetPortIter = port.FindMember("targetPort");
        ports[index] = std::make_unique<ServicePort>();
        ports[index]->port = portIter->value.GetUint();
        ports[index]->targetPort = targetPortIter->value.GetUint();
        ports[index]->protocol = protocolIter->value.GetString();
        index++;
    }

    if (spec->value.HasMember("selector")) {
        auto selectorIter = spec->value.FindMember("selector");
        if (!selectorIter->value.IsObject()) {
            return false;
        }
        auto selectorMapObject = selectorIter->value.GetObject();
        for (auto iter = selectorMapObject.begin(); iter != selectorMapObject.end(); iter++) {
            selector[iter->name.GetString()] = iter->value.GetString();
        }
    }


    if (!spec->value.HasMember("clusterIP")) {
        return false;
    }
    auto clusterIter = spec->value.FindMember("clusterIP");
    clusterIP = clusterIter->value.GetString();

    if (!spec->value.HasMember("clusterIPs")) {
        return false;
    }
    auto clusterIPsIter = spec->value.FindMember("clusterIPs");
    auto clusterIPsIterArray = clusterIPsIter->value.GetArray();
    clusterIPs.resize(clusterIPsIterArray.Size());
    index = 0;
    for (auto &clusterIPData : clusterIPsIterArray) {
        clusterIPs[index] = clusterIPData.GetString();
    }

    if (!spec->value.HasMember("type")) {
        return false;
    }
    type = spec->value.FindMember("type")->value.GetString();

    if (!spec->value.HasMember("sessionAffinity")) {
        return false;
    }
    auto sessionAffinityIter = spec->value.FindMember("sessionAffinity");
    sessionAffinity = sessionAffinityIter->value.GetString();

    if (!spec->value.HasMember("ipFamilies")) {
        return false;
    }
    auto ipFamiliesIter = spec->value.FindMember("ipFamilies");
    if (!ipFamiliesIter->value.IsArray()) {
       return false;
    }
    auto ipFamiliesArray = ipFamiliesIter->value.GetArray();
    index = 0;
    ipFamilies.resize(ipFamiliesArray.Size());
    for (auto &ipFamilie : ipFamiliesArray) {
        ipFamilies[index] = ipFamilie.GetString();
        index++;
    }

    if (!spec->value.HasMember("ipFamilyPolicy")) {
        return false;
    }
    if (!spec->value.FindMember("ipFamilyPolicy")->value.IsString()) {
        return false;
    }
    ipFamilyPolicy = spec->value.FindMember("ipFamilyPolicy")->value.GetString();

    if (!spec->value.HasMember("internalTrafficPolicy")) {
        return false;
    }
    if (!spec->value.FindMember("internalTrafficPolicy")->value.IsString()) {
        return false;
    }
    internalTrafficPolicy = spec->value.FindMember("internalTrafficPolicy")->value.GetString();
    toPeer();
    return true;
}

const std::vector<std::string>& Service::toPeer() {
    if (!peers.empty()) {
        return peers;
    }
    // demo-order-service.demo-shopping.svc.cluster.local:8080
    std::string domain =  fmt::format("{}.{}.svc.cluster.local", name, namespaceValue);
    peers.emplace_back(domain);
    for (auto &ip : clusterIPs) {
        peers.emplace_back(ip);
        for (auto &port : ports) {
            auto endpoint = fmt::format("{}:{}", ip, port->port);
            peers.emplace_back(endpoint);
            auto domainPort = fmt::format("{}:{}", domain, port->port);
            peers.emplace_back(domainPort);
        }
    }

    return peers;
}
}