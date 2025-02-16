//
// Created by zhanglei on 2025/2/16.
//
#pragma once

#include <memory>

#include <component/api.h>
#include "listener_manager.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
class Processor : public Core::Component::Interceptor {
public:
    Processor();

    Core::Component::Result intercept(std::unique_ptr<Core::Component::Batch>& batch) final;

    Core::Component::Result intercept(Core::Component::Batch& batch) final;

    ~Processor() {

    };
private:
    std::unique_ptr<ListenerManager> listenerManager;
};
}
