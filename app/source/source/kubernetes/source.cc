//
// Created by zhanglei on 2025/2/24.
//
#include "app/include/source/kubernetes/source.h"

namespace App::Source::Kubernetes {
void Source::init() {
    if (!config_->GetConfig().has_kubernetes_discovery_config()) {
        return;
    }
    if (!config_->GetConfig().kubernetes_discovery_config().enable()) {
        return;
    }
    watcher->init();
}

void Source::start() {
    if (!config_->GetConfig().has_kubernetes_discovery_config()) {
        return;
    }
    if (!config_->GetConfig().kubernetes_discovery_config().enable()) {
        return;
    }
    watcher->start();
};

void Source::stop() {
    if (!config_->GetConfig().has_kubernetes_discovery_config()) {
        return;
    }
    if (!config_->GetConfig().kubernetes_discovery_config().enable()) {
        return;
    }
    watcher->stop();
};

void Source::finish() {

};

std::string Source::name(){
    return "kubernetes";
}

Source::~Source() {

}

}