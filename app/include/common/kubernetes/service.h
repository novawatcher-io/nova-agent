//
// Created by zhanglei on 2025/2/25.
//

#pragma once

#include <memory>
#include <vector>
#include <map>

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

namespace App::Common::Kubernetes {

class ServicePort {
public:
    std::string protocol;
    uint16_t port;
    uint16_t targetPort;
};

class Service {
public:
    Service() = default;
    ~Service() = default;
    const std::vector<std::string>& toPeer();
    bool parseFromString(rapidjson::Document& document);

    std::string name;
    std::string namespaceValue;
    std::vector<std::unique_ptr<ServicePort>> ports;
    std::map<std::string, std::string> selector;
    std::string clusterIP;
    std::vector<std::string> clusterIPs;
    std::string type;
    std::string sessionAffinity;
    std::vector<std::string> ipFamilies;
    std::string ipFamilyPolicy;
    std::string internalTrafficPolicy;
    std::vector<std::string> peers;
};
}