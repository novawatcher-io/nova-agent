//
// Created by zhanglei on 2025/2/16.
//
#pragma once

#include <memory>

#include <component/api.h>
#include <component/thread_container.h>
#include <component/timer_channel.h>
#include "listener_manager.h"
#include "app/include/sink/topology/sink.h"
#include "config/nova_agent_config.h"
#include <event/event_loop.h>

namespace App::Intercept::Opentelemetry::Trace::Topology {
class Processor : public Core::Component::Interceptor {
public:
    explicit Processor(std::shared_ptr<App::Config::ConfigReader> config,
       const std::shared_ptr<Core::Event::EventLoop>& loop_);

    Core::Component::Result intercept(std::unique_ptr<Core::Component::Batch>& batch) final;

    Core::Component::Result intercept(Core::Component::Batch& batch) final;

    void start() final;

    void stop() final;

    ~Processor() {

    };
private:
    // 刷新指标
    void flushMetric();
    std::unique_ptr<ListenerManager> listenerManager;
    std::unique_ptr<Sink::Topology::Sink> sink;
    std::shared_ptr<Core::Component::TimerChannel> timer_;
    const std::shared_ptr<Core::Event::EventLoop>& loop;
};
}
