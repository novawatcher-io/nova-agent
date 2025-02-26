//
// Created by zhanglei on 2025/2/26.
//

#pragma once
#include "non_copyable.h"
#include "non_moveable.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

namespace App::Common::Kubernetes {
class Endpoint :public Core::Nonmoveable, public Core::Noncopyable {
public:
    bool parseFromString(rapidjson::Document& document);

    std::string name;
    std::string namespaceValue;
    std::string podIp;
};
}
