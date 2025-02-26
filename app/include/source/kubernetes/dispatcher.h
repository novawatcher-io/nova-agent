//
// Created by zhanglei on 2025/2/26.
//

#pragma once

#include <core.h>
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

namespace App::Source::Kubernetes {
class Dispatcher {
public:
    virtual void dispatch(rapidjson::Document& doc) PURE;

    virtual ~Dispatcher() {};
};
}
