//
// Created by zhanglei on 2025/2/26.
//
#include "app/include/common/kubernetes/endpoint.h"

namespace App::Common::Kubernetes {

bool Endpoint::parseFromString(rapidjson::Document &document) {
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
    if (!metadata->value.FindMember("namespace")->value.IsString()) {
        return false;
    }
    namespaceValue = metadata->value.FindMember("namespace")->value.GetString();
    if (!document.FindMember("object")->value.HasMember("subsets")) {
        return false;
    }

    if (!document.FindMember("object")->value.FindMember("subsets")->value.IsArray()) {
        return false;
    }
    auto subsets = document.FindMember("object")->value.FindMember("subsets")->value.GetArray();
    for (auto& subset : subsets) {
        if (!subset.HasMember("addresses")) {
            continue;
        }
        if (!subset.FindMember("addresses")->value.IsArray()) {
            continue;
        }
        auto addressesArray = subset.FindMember("addresses")->value.GetArray();
        for (auto& address : addressesArray) {
            if (!address.IsObject()) {
                continue;
            }
            if (!address.HasMember("ip")) {
                continue;
            }
            if (!address.FindMember("ip")->value.IsString()) {
                continue;
            }
            addresses.emplace_back(address.FindMember("ip")->value.GetString());
        }
    }
    return true;
}
}