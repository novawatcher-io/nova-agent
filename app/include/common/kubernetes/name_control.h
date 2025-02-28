//
// Created by zhanglei on 2025/2/25.
//

#pragma once

#include <map>
#include <vector>
#include <shared_mutex>
#include <atomic>
#include <mutex>
#include <utility>

#include "service.h"
#include "endpoint.h"
#include "pod.h"

namespace App::Common::Kubernetes {


class NameControl {
public:
    NameControl();


    void set(std::unique_ptr<Service> service);

    void remove(std::unique_ptr<Service>& service);

    Service* get(std::string peer);

    void set(std::unique_ptr<Pod> endpoint);

    void remove(std::unique_ptr<Pod>& endpoint);

    std::pair<std::string, std::string> findServiceName(std::string peer);

    void set(std::unique_ptr<Endpoint> endpoint);

    void remove(std::unique_ptr<Endpoint>& endpoint);

        // 提供一个公共的静态方法来获取单例对象
    static NameControl* getInstance();

private:
    std::map<std::string, std::unique_ptr<Service>> services;
    std::map<std::string, std::unique_ptr<Endpoint>> endpoints;

    std::map<std::string, Service*> peerToServices;

    static NameControl* m_instance;

    std::map<std::string, std::unique_ptr<Pod>> podIpMap;

    std::map<std::string, Endpoint*> ipToEndpoints;

    std::shared_mutex mtx;
};

}
