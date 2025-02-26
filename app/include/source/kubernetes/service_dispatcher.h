//
// Created by zhanglei on 2025/2/26.
//

#pragma once

#include <core.h>
#include "dispatcher.h"

namespace App::Source::Kubernetes {
class ServiceDispatcher :public Core::Noncopyable, public Core::Nonmoveable, public Dispatcher {
public:
    ServiceDispatcher();

    void dispatch(rapidjson::Document& doc);

    virtual ~ServiceDispatcher();
};
}
