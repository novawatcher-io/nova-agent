/**
******************************************************************************
* @file           : source.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/2/20
******************************************************************************
*/
#pragma once

#include <memory>
#include <string>
#include <event/event_loop.h>

#include "app/include/common/const.h"
#include "app/include/common/base_thread.h"
#include "app/include/prometheus/prometheus_exposer.h"
#include "callback/configuration_discovery_server.h"
#include "callback/jvm_metric_report_server.h"
#include "callback/log_report_server.h"
#include "callback/management_server.h"
#include "callback/meter_report_server.h"
#include "callback/trace_server.h"
#include <component/api.h>
#include <grpcpp/grpcpp.h>
#include "config/nova_agent_config.h"

namespace App {
namespace Sink {
namespace Channel {
class GrpcChannel;
}
} // namespace Sink
} // namespace App

namespace App::Source::SkyWalking::Grpc {
class Source : public Core::Component::Component {
public:
    Source(std::shared_ptr<App::Config::ConfigReader> config,
       std::unique_ptr<App::Prometheus::PrometheusExposer>& exposer,
       const std::shared_ptr<Core::Event::EventLoop>& loop_)
    : exposer_(exposer), config_(config), loop(loop_) {
    }

    std::string name() override {
        return App::Common::Trace::Skywalking::skywalking_grpc;
    }

    void init() final;

    void start() final;

    void stop() final;

    void finish() final;

    std::unique_ptr<grpc::CompletionQueue>& getCq() {
        return cq_;
    }

    virtual ~Source() {};

private:
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<grpc::CompletionQueue> cq_;
    std::unique_ptr<Core::Component::Pipeline> logPipeline;
    std::unique_ptr<Core::Component::Pipeline> metricPipeline;
    Core::Component::Pipeline* tracePipelinePtr = nullptr;

    grpc::ServerBuilder builder;
    std::unique_ptr<Callback::ConfigurationDiscoveryServer> configurationDiscoveryService;
    std::unique_ptr<Callback::JvmMetricReportServer> jvmMetricReportService;
    std::unique_ptr<Callback::LogReportServer> logReportService;
    std::unique_ptr<Callback::MeterReportServer> meterReportService;
    std::unique_ptr<Callback::ManagementServer> managementService;
    std::unique_ptr<Callback::TraceServer> traceServer;
    std::unique_ptr<App::Prometheus::PrometheusExposer>& exposer_;
    std::shared_ptr<App::Config::ConfigReader> config_;
    const std::shared_ptr<Core::Event::EventLoop>& loop;
};
} // namespace App::Source::SkyWalking::Grpc