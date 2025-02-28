//
// Created by zhanglei on 2025/2/24.
//
#include "app/include/source/kubernetes/watcher.h"

#include <chrono>
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <spdlog/spdlog.h>
#include <curl/curl.h>
#include <openssl/crypto.h>

#include "app/include/source/kubernetes/service_dispatcher.h"
#include "app/include/source/kubernetes/pods_dispatcher.h"
#include "app/include/source/kubernetes/endpoint_dispatcher.h"
#include "app/include/source/kubernetes/const.h"

namespace App::Source::Kubernetes {

static std::map<std::string, std::unique_ptr<Dispatcher>> dispatchers;


Watcher::Watcher(std::shared_ptr<App::Config::ConfigReader>& config) :config_(config) {
    if (config->GetConfig().has_kubernetes_discovery_config()) {
        size_t size = config->GetConfig().kubernetes_discovery_config().kube_config().size();
        basePath = (char*)malloc(size);
        memcpy(basePath, config->GetConfig().kubernetes_discovery_config().kube_config().c_str(), size);
    }
    runnerServiceThread = std::make_unique<App::Common::BaseThread>();
    runnerPodsThread = std::make_unique<App::Common::BaseThread>();
    runnerEndpointsThread = std::make_unique<App::Common::BaseThread>();
}

std::mutex mutex;
std::condition_variable cond;
bool ready = true;
//// 多个线程运行watcher程序，由于openssl存在bug，多线程下会出现问题，所以用条件变量控制，每次只会运行一个watcher，跑起来后再运行另一个
//void run_wait() {
//    std::unique_lock<std::mutex> lck(mutex);
//    while (!ready) {
//        cond.wait(lck);
//    }
//    ready = false;
//}
//
//void run_ready() {
//    std::lock_guard<std::mutex> guard(mutex);
//    ready = true;
//    std::cout << "aaa" << std::endl;
//    cond.notify_all();
//}

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

void my_watch_handler(void **pData, long *pDataLen)
{
    kubernets_watch_handler(pData, pDataLen, on_pod_event_comes);
}


void Watcher::watch_list_endpoints(apiClient_t * apiClient)
{

    int watch = 1;

    CoreV1API_listEndpointsForAllNamespaces(apiClient,   /*namespace */
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

std::atomic<bool> is_shutdown_ = false;



static int my_watch_progress_func(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    if (is_shutdown_ == true) {
        return 1;
    }
    return 0;
}
void Watcher::watch_list_pods(apiClient_t * apiClient)
{
    int watch = 1;
    // int timeoutSeconds = 0;  /* Set timeoutSeconds to 0 to keep watching and not exit */
    CoreV1API_listPodForAllNamespaces(apiClient,   /*namespace */
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

void Watcher::watch_list_service(apiClient_t * apiClient)
{

    int watch = 1;
    // int timeoutSeconds = 0;  /* Set timeoutSeconds to 0 to keep watching and not exit */
    CoreV1API_listServiceForAllNamespaces(apiClient,   /*namespace */
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
    int rc = load_kube_config(&basePath, (&sslConfig), &apiKeys, nullptr);   /* NULL means loading configuration from $HOME/.kube/config */
    if (rc != 0) {
        printf("Cannot load kubernetes configuration.\n");
        return;
    }


}



void Watcher::start() {
    dispatchers[Kind::service] = std::make_unique<ServiceDispatcher>();
    dispatchers[Kind::pod] = std::make_unique<PodsDispatcher>();
    dispatchers[Kind::endpoint] = std::make_unique<EndpointDispatcher>();
  /* Must initialize libcurl before any threads are started */
    apiClient_setupGlobalEnv();
    apiClient = apiClient_create_with_base_path(basePath, sslConfig, apiKeys);
    apiClient->progress_func = my_watch_progress_func;
    apiClient->data_callback_func = my_watch_handler;
    runnerServiceThread->addInitCallable([this] {
        while (!is_shutdown_) {
            watch_list_service(apiClient);
            watch_list_endpoints(apiClient);
            watch_list_pods(apiClient);
        }

    });
    runnerServiceThread->start();
    return;
}

void Watcher::stop() {
    is_shutdown_ = true;
    apiClient_free(apiClient);
    free_client_config(basePath, sslConfig, apiKeys);
    basePath = NULL;
    sslConfig = NULL;
    apiKeys = NULL;
    runnerServiceThread->stop();
    runnerEndpointsThread->stop();
    runnerPodsThread->stop();
//    apiClient_unsetupGlobalEnv();

}
}
