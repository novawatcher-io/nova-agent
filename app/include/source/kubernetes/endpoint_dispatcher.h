//
// Created by zhanglei on 2025/2/26.
//

#pragma once

#include "dispatcher.h"

namespace App::Source::Kubernetes {
class EndpointDispatcher :public Core::Noncopyable, public Core::Nonmoveable, public Dispatcher {
public:
    EndpointDispatcher();

    void dispatch(rapidjson::Document& doc);

    virtual ~EndpointDispatcher();
};
}