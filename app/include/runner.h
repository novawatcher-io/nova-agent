#pragma once
#include "app/include/common/base_thread.h"
#include "config/nova_agent_config.h"
#include <component/container.h>
#include <event/event_loop.h>
#include <event2/util.h>
#include <grpcpp/grpcpp.h>
#include <memory>

namespace App {
class Runner {
public:
    Runner(std::shared_ptr<App::Config::ConfigReader> config)
        : loop(std::make_unique<Core::Event::EventLoop>()),
          sourceThread(std::make_shared<App::Common::BaseThread>()),
          cq(std::make_unique<grpc::CompletionQueue>()),
          config_(config) {
    }
    void run();

private:
    static void onstop(evutil_socket_t sig, short events, void* param);

    void initMetricExporter();

    void cleanupMetrics();

    /**
     * @brief 事件循环
     *
     */
    std::shared_ptr<Core::Event::EventLoop> loop;

    std::shared_ptr<Core::Component::Container> manager;

    std::shared_ptr<App::Common::BaseThread> sourceThread;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    std::unique_ptr<grpc::CompletionQueue> cq;
    std::shared_ptr<App::Config::ConfigReader> config_;
};
} // namespace App
