//
// Created by zhanglei on 2025/2/26.
//
#include "app/include/source/kubernetes/pods_dispatcher.h"

#include "app/include/common/kubernetes/endpoint.h"
#include "app/include/common/kubernetes/name_control.h"
#include "app/include/source/kubernetes/const.h"

namespace App::Source::Kubernetes {
PodsDispatcher::PodsDispatcher() {}

void PodsDispatcher::dispatch(rapidjson::Document &doc) {
    std::unique_ptr<Common::Kubernetes::Endpoint> endpoint = std::make_unique<Common::Kubernetes::Endpoint>();
    bool ret = endpoint->parseFromString(doc);
    if (!ret) {
        SPDLOG_ERROR("parse endpoint error: {}", doc.GetString());
        return;
    }
    if (!doc.HasMember(type.c_str())) {
        return;
    }

    auto typeIter = doc.FindMember(type.c_str());
    if (typeIter->value.GetString() == added) {
        Common::Kubernetes::NameControl::getInstance()->set(std::move(endpoint));
        return;
    }

    if (typeIter->value.GetString() == modified) {
        Common::Kubernetes::NameControl::getInstance()->set(std::move(endpoint));
        return;
    }

    if (typeIter->value.GetString() == deleted) {
        Common::Kubernetes::NameControl::getInstance()->remove(endpoint);
        return;
    }

    if (typeIter->value.GetString() == bookmark) {
        SPDLOG_DEBUG("event_string is not a valid json: {}", event_string);
        return;
    }

    if (typeIter->value.GetString() == error) {
        SPDLOG_DEBUG("event_string is not a valid json: {}", event_string);
        return;
    }
}

PodsDispatcher::~PodsDispatcher() {}
}