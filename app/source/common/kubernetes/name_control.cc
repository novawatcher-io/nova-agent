//
// Created by zhanglei on 2025/2/25.
//
#include "app/include/common/kubernetes/name_control.h"
namespace App::Common::Kubernetes {
NameControl* NameControl::m_instance = nullptr;
static std::once_flag flag;

NameControl * NameControl::getInstance() {
    std::call_once(flag, [](){
        m_instance = new NameControl();
    });
    return m_instance;
}

NameControl::NameControl() {

}

void NameControl::set(std::unique_ptr<Service> service) {
    std::lock_guard<std::shared_mutex> wk(mtx);
    auto name = service->name;
    auto iter = services.find(service->name);
    auto peers = service->toPeer();
    if (iter != services.end()) {
        for (auto &peer : peers) {
            auto peerIter = peerToServices.find(peer);
            if (peerIter != peerToServices.end()) {
                peerToServices.erase(peerIter);
            }
        }
        services.erase(iter);
    }
    if (peers.empty()) {
        return;
    }
    for (auto &peer : peers) {
        peerToServices[peer] = service.get();
    }
    services[service->name] = std::move(service);
    return;
}

void NameControl::remove(std::unique_ptr<Service>& service) {
    std::lock_guard<std::shared_mutex> wk(mtx);
    auto iter = services.find(service->name);
    if (iter == services.end()) {
        return;
    }
    auto peers = service->toPeer();
    for (auto &peer : peers) {
        auto peerIter = peerToServices.find(peer);
        if (peerIter != peerToServices.end()) {
            peerToServices.erase(peerIter);
        }
    }
    services.erase(iter);
    return;
}

Service* NameControl::get(std::string peer) {
    std::shared_lock<std::shared_mutex> rk(mtx);
    auto serviceIter = peerToServices.find(peer);
    if (serviceIter == peerToServices.end()) {
        return nullptr;
    }
    return serviceIter->second;
}

void NameControl::set(std::unique_ptr<Pod> pod) {
    std::lock_guard<std::shared_mutex> wk(mtx);
    podIpMap[pod->podIp] = std::move(pod);
}

void NameControl::remove(std::unique_ptr<Pod>& endpoint) {
    std::lock_guard<std::shared_mutex> wk(mtx);
    auto iter = podIpMap.find(endpoint->podIp);
    if (iter == podIpMap.end()) {
        return;
    }
    podIpMap.erase(iter);
}

std::pair<std::string, std::string> NameControl::findServiceName(std::string peer) {
    auto servicePtr = peerToServices.find(peer);
    if (servicePtr != peerToServices.end()) {
        return {servicePtr->second->name, servicePtr->second->namespaceValue};
    }

    auto endpointIter = ipToEndpoints.find(peer);
    if (endpointIter != ipToEndpoints.end()) {
        return {endpointIter->second->name, endpointIter->second->namespaceValue};
    }

    auto endpointItr = podIpMap.find(peer);
    if (endpointItr != podIpMap.end()) {
        return {endpointItr->second->name, endpointItr->second->namespaceValue};
    }
    return {};
}

void NameControl::set(std::unique_ptr<Endpoint> endpoint) {
    std::lock_guard<std::shared_mutex> wk(mtx);
    auto iter = endpoints.find(endpoint->name);
    auto ptr = endpoint.get();
    if (iter != endpoints.end()) {
        for (auto& ip : iter->second->addresses) {
            auto ipIter = ipToEndpoints.find(ip);
            if (ipIter != ipToEndpoints.end()) {
                ipToEndpoints.erase(ipIter);
            }
        }
        endpoints.erase(iter);
    }
    endpoints[endpoint->name] = std::move(endpoint);
    for (auto& ip : ptr->addresses) {
        ipToEndpoints[ip] = ptr;
    }
}

void NameControl::remove(std::unique_ptr<Endpoint>& endpoint) {
    std::lock_guard<std::shared_mutex> wk(mtx);
    auto iter = endpoints.find(endpoint->name);
    if (iter != endpoints.end()) {
        for (auto& ip : iter->second->addresses) {
            auto ipIter = ipToEndpoints.find(ip);
            if (ipIter != ipToEndpoints.end()) {
                ipToEndpoints.erase(ipIter);
            }
        }
        endpoints.erase(iter);
    }
}
}