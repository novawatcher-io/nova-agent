//
// Created by zhanglei on 2025/2/24.
//
#include "app/include/source/kubernetes/service_watcher.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <spdlog/spdlog.h>

#include "app/include/common/kubernetes/service.h"
#include "app/include/source/kubernetes/const.h"

namespace App::Source::Kubernetes {

ServiceWatcher::ServiceWatcher(std::shared_ptr<App::Config::ConfigReader>& config) {
    if (config->GetConfig().has_kubernetes_discovery_config()) {
        basePath = config->GetConfig().kubernetes_discovery_config().kube_config();
    }
    if (basePath == "") {
        basePath = "~/.kube/config";
    }
}

void on_pod_event_comes(const char *event_string)
{
    rapidjson::Document doc;
    if (doc.Parse(event_string).HasParseError()) {
        SPDLOG_ERROR("event_string is not a valid json: {}", event_string);
        return;
    }

    if (!doc.IsObject()) {
        SPDLOG_ERROR("event_string is not an array: {}", event_string);
        return;
    }
    std::unique_ptr<Common::Kubernetes::Service> service = std::make_unique<Common::Kubernetes::Service>();
    bool ret = service->parseFromString(doc);
    if (!ret) {
        SPDLOG_ERROR("parse server error: {}", event_string);
        return;
    }
    if (!doc.HasMember(type.c_str())) {
        return;
    }

    auto typeIter = doc.FindMember(type.c_str());
    if (typeIter->value.GetString() == added) {
        return;
    }

    if (typeIter->value.GetString() == modified) {
        return;
    }

    if (typeIter->value.GetString() == deleted) {
        return;
    }

    if (typeIter->value.GetString() == bookmark) {
        return;
    }

    if (typeIter->value.GetString() == error) {
        return;
    }
}

void my_pod_watch_handler(void **pData, long *pDataLen)
{
    kubernets_watch_handler(pData, pDataLen, on_pod_event_comes);
}

void ServiceWatcher::watch_list_service(apiClient_t * apiClient)
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
}

void ServiceWatcher::init() {
    auto path = (char *)basePath.c_str();
    int rc = load_kube_config(&path, &sslConfig, &apiKeys, nullptr);   /* NULL means loading configuration from $HOME/.kube/config */
    if (rc != 0) {
        printf("Cannot load kubernetes configuration.\n");
        return;
    }
     apiClient = apiClient_create_with_base_path(path, sslConfig, apiKeys);
    if (!apiClient) {
        printf("Cannot create a kubernetes client.\n");
        return;
    }

}

void ServiceWatcher::start() {
    watch_list_service(apiClient);
}

void ServiceWatcher::stop() {
    apiClient_free(apiClient);
    apiClient = NULL;
    free_client_config((char *)basePath.c_str(), sslConfig, apiKeys);
    sslConfig = NULL;
    apiKeys = NULL;
    apiClient_unsetupGlobalEnv();
}
}
