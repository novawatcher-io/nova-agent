//
// Created by zhanglei on 2025/2/24.
//

#pragma once

#include <atomic>

extern "C" {
#include <kubernetes/config/kube_config.h>
#include <kubernetes/include/apiClient.h>
#include <kubernetes/api/CoreV1API.h>
#include <kubernetes/watch/watch_util.h>
}

#include "os/unix_countdown_latch.h"
#include "dispatcher.h"
#include "common/base_thread.h"
#include "config/nova_agent_config.h"

namespace App::Source::Kubernetes {
class Watcher {
public:
    Watcher(std::shared_ptr<App::Config::ConfigReader>& config);

    void init();

    void start();

    void stop();

    void watch_list_service(apiClient_t * apiClient);

    void watch_list_endpoints(apiClient_t * apiClient);

    void watch_list_pods(apiClient_t * apiClient);
private:
    std::unique_ptr<App::Common::BaseThread> runnerServiceThread;
    char* basePath = nullptr;
    sslConfig_t *sslConfig = nullptr;
    list_t *apiKeys = nullptr;
    std::shared_ptr<App::Config::ConfigReader>& config_;
    apiClient_t *apiClient = nullptr;
    int timeoutSeconds = 10;    /* Watch for 30 seconds */
};
}
