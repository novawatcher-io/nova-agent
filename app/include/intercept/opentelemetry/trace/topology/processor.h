//
// Created by zhanglei on 2025/2/16.
//
#pragma once

#include <memory>

#include <component/api.h>
#include "listener_manager.h"
#include "app/include/sink/topology/sink.h"
#include "config/nova_agent_config.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
class Processor : public Core::Component::Interceptor {
public:
    explicit Processor(std::shared_ptr<App::Config::ConfigReader> config);

    Core::Component::Result intercept(std::unique_ptr<Core::Component::Batch>& batch) final;

    Core::Component::Result intercept(Core::Component::Batch& batch) final;

    ~Processor() {

    };
private:
    std::unique_ptr<ListenerManager> listenerManager;
    std::unique_ptr<Sink::Topology::Sink> sink;
};
}
