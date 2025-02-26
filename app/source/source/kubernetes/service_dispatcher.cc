//
// Created by zhanglei on 2025/2/26.
//

#include "app/include/source/kubernetes/service_dispatcher.h"

#include <spdlog/spdlog.h>

#include "app/include/common/kubernetes/service.h"
#include "app/include/source/kubernetes/const.h"
#include "app/include/common/kubernetes/name_control.h"

namespace App::Source::Kubernetes {
ServiceDispatcher::ServiceDispatcher() {}

void ServiceDispatcher::dispatch(rapidjson::Document &doc) {
    std::unique_ptr<Common::Kubernetes::Service> service = std::make_unique<Common::Kubernetes::Service>();
    bool ret = service->parseFromString(doc);
    if (!ret) {
        SPDLOG_ERROR("parse server error: {}", doc.GetString());
        return;
    }
    if (!doc.HasMember(type.c_str())) {
        return;
    }

    auto typeIter = doc.FindMember(type.c_str());
    auto typeStr = typeIter->value.GetString();
    if (typeStr == added) {
        Common::Kubernetes::NameControl::getInstance()->set(std::move(service));
        return;
    }

    if (typeIter->value.GetString() == modified) {
        Common::Kubernetes::NameControl::getInstance()->set(std::move(service));
        return;
    }

    if (typeIter->value.GetString() == deleted) {
        Common::Kubernetes::NameControl::getInstance()->remove(service);
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

ServiceDispatcher::~ServiceDispatcher() {}
}