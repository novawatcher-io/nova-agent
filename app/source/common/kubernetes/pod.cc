//
// Created by zhanglei on 2025/2/26.
//
#include "app/include/common/kubernetes/pod.h"

namespace App::Common::Kubernetes {

bool Pod::parseFromString(rapidjson::Document &document) {
    if (!document.HasMember("object")) {
        return false;
    }

    if (!document.FindMember("object")->value.IsObject()) {
        return false;
    }

    if (!document.FindMember("object")->value.HasMember("metadata")) {
        return false;
    }

    if (!document.FindMember("object")->value.FindMember("metadata")->value.IsObject()) {
        return false;
    }

    if (!document.FindMember("object")->value.FindMember("metadata")->value.HasMember("name")) {
        return false;
    }

    auto metadata = document.FindMember("object")->value.FindMember("metadata");

    name = metadata->value.FindMember("name")->value.GetString();
    if (!metadata->value.HasMember("namespace")) {
        return false;
    }
    if (!document.FindMember("object")->value.HasMember("status")) {
        return false;
    }
    auto status = document.FindMember("object")->value.FindMember("status");
    if (!status->value.IsObject()) {
        return false;
    }
    if (!status->value.HasMember("podIP")) {
        return false;
    }
    if (!status->value.FindMember("podIP")->value.IsString()) {
        return false;
    }
    podIp = status->value.FindMember("podIP")->value.GetString();
    namespaceValue = metadata->value.FindMember("namespace")->value.GetString();
    return true;
}
}