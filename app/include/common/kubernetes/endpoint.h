//
// Created by zhanglei on 2025/2/28.
//

#pragma once
#include "non_copyable.h"
#include "non_moveable.h"

#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

namespace App::Common::Kubernetes {
class Endpoint :public Core::Nonmoveable, public Core::Noncopyable {
public:
    Endpoint() = default;
    bool parseFromString(rapidjson::Document &document);
    ~Endpoint() = default;

    // Service Name
    std::string name;
    std::string namespaceValue;
    std::vector<std::string>addresses;
};
}