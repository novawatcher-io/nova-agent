//
// Created by zhanglei on 2025/2/24.
//

#pragma once

extern "C" {
#include <kubernetes/config/kube_config.h>
#include <kubernetes/include/apiClient.h>
#include <kubernetes/api/CoreV1API.h>
#include <kubernetes/watch/watch_util.h>
}

#include "config/nova_agent_config.h"

namespace App::Source::Kubernetes {
class ServiceWatcher {
public:
    ServiceWatcher(std::shared_ptr<App::Config::ConfigReader>& config);

    void init();

    void start();

    void stop();

    void watch_list_service(apiClient_t * apiClient);
private:
    std::string basePath;
    sslConfig_t *sslConfig = nullptr;
    list_t *apiKeys = nullptr;
    apiClient_t *apiClient = nullptr;
};
}
