//
// Created by zhanglei on 2025/2/24.
//
#include "app/include/source/kubernetes/watcher.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <spdlog/spdlog.h>

#include "app/include/source/kubernetes/service_dispatcher.h"
#include "app/include/source/kubernetes/pods_dispatcher.h"
#include "app/include/source/kubernetes/const.h"

namespace App::Source::Kubernetes {

static std::map<std::string, std::unique_ptr<Dispatcher>> dispatchers;


Watcher::Watcher(std::shared_ptr<App::Config::ConfigReader>& config) {
    if (config->GetConfig().has_kubernetes_discovery_config()) {
        size_t size = config->GetConfig().kubernetes_discovery_config().kube_config().size();
        basePath = (char*)malloc(size);
        memcpy(basePath, config->GetConfig().kubernetes_discovery_config().kube_config().c_str(), size);
    }
    runnerThread = std::make_unique<App::Common::BaseThread>();
}

void on_pod_event_comes(const char *event_string)
{
    rapidjson::Document doc;
    SPDLOG_INFO("event_string is json: {}", event_string);
    if (doc.Parse(event_string).HasParseError()) {
        SPDLOG_ERROR("event_string is not a valid json: {}", event_string);
        return;
    }

    if (!doc.IsObject()) {
        SPDLOG_ERROR("event_string is not an array: {}", event_string);
        return;
    }

    if (!doc.HasMember("object")) {
        SPDLOG_ERROR("event_string not have object: {}", event_string);
        return;
    }

    if (!doc.FindMember("object")->value.IsObject()) {
        SPDLOG_ERROR("event_string is not an object: {}", event_string);
        return;
    }

    if (!doc.FindMember("object")->value.HasMember("kind")) {
        SPDLOG_ERROR("event_string not have kind: {}", event_string);
        return;
    }

    if (!doc.FindMember("object")->value.FindMember("kind")->value.IsString()) {
        SPDLOG_ERROR("event_string kind is not an string: {}", event_string);
        return;
    }
    auto kind = doc.FindMember("object")->value.FindMember("kind")->value.GetString();
    auto dispatcherIter = dispatchers.find(kind);
    if (dispatcherIter == dispatchers.end()) {
        SPDLOG_ERROR("event_string kind is not register dispatcher: {}", event_string);
        return;
    }
    dispatcherIter->second->dispatch(doc);
}

void my_pod_watch_handler(void **pData, long *pDataLen)
{
    kubernets_watch_handler(pData, pDataLen, on_pod_event_comes);
}

void Watcher::watch_list_service(apiClient_t * apiClient)
{
    apiClient->data_callback_func = my_pod_watch_handler;

    int watch = 1;
    int timeoutSeconds = 30;    /* Watch for 30 seconds */
    // int timeoutSeconds = 0;  /* Set timeoutSeconds to 0 to keep watching and not exit */
    CoreV1API_listNamespacedService(apiClient, "",   /*namespace */
                                NULL,   /* pretty */
                                NULL,   /* allowWatchBookmarks */
                                NULL,   /* continue */
                                NULL,   /* fieldSelector */
                                NULL,   /* labelSelector */
                                NULL,   /* limit */
                                NULL,   /* resourceVersion */
                                NULL,   /* resourceVersionMatch */
                                NULL,   /* sendInitialEvents */
                                &timeoutSeconds,    /* timeoutSeconds */
                                &watch  /* watch */
        );

    CoreV1API_listNamespacedPod(apiClient, "",   /*namespace */
                                NULL,   /* pretty */
                                NULL,   /* allowWatchBookmarks */
                                NULL,   /* continue */
                                NULL,   /* fieldSelector */
                                NULL,   /* labelSelector */
                                NULL,   /* limit */
                                NULL,   /* resourceVersion */
                                NULL,   /* resourceVersionMatch */
                                NULL,   /* sendInitialEvents */
                                &timeoutSeconds,    /* timeoutSeconds */
                                &watch  /* watch */
        );
}

void Watcher::init() {
    int rc = load_kube_config(&basePath, &sslConfig, &apiKeys, nullptr);   /* NULL means loading configuration from $HOME/.kube/config */
    if (rc != 0) {
        printf("Cannot load kubernetes configuration.\n");
        return;
    }
     apiClient = apiClient_create_with_base_path(basePath, sslConfig, apiKeys);
    if (!apiClient) {
        printf("Cannot create a kubernetes client.\n");
        return;
    }

}

void Watcher::start() {
    dispatchers[Kind::service] = std::make_unique<ServiceDispatcher>();
    dispatchers[Kind::pod] = std::make_unique<PodsDispatcher>();
    runnerThread->addInitCallable([this] {
        while(!is_shutdown_.load(std::memory_order_acquire)) {
            watch_list_service(apiClient);
        }
    });
    runnerThread->start();
    return;
}

void Watcher::stop() {
    is_shutdown_ = true;
    apiClient_free(apiClient);
    apiClient = NULL;
    free_client_config(basePath, sslConfig, apiKeys);
    sslConfig = NULL;
    apiKeys = NULL;
    apiClient_unsetupGlobalEnv();
    runnerThread->stop();
}
}
