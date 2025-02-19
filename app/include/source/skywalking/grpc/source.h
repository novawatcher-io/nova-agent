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
#include <memory>
#include <string>
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
    Source(std::shared_ptr<App::Config::ConfigReader> config, std::unique_ptr<App::Prometheus::PrometheusExposer>& exposer)
    : exposer_(exposer) {
        channelThread = std::make_shared<App::Common::BaseThread>();
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

private:
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<grpc::CompletionQueue> cq_;
    std::shared_ptr<Sink::Channel::GrpcChannel> channel;
    std::unique_ptr<Core::Component::Pipeline> logPipeline;
    std::unique_ptr<Core::Component::Pipeline> metricPipeline;
    std::unique_ptr<Core::Component::Pipeline> tracePipeline;

    grpc::ServerBuilder builder;
    std::unique_ptr<Callback::ConfigurationDiscoveryServer> configurationDiscoveryService;
    std::unique_ptr<Callback::JvmMetricReportServer> jvmMetricReportService;
    std::unique_ptr<Callback::LogReportServer> logReportService;
    std::unique_ptr<Callback::MeterReportServer> meterReportService;
    std::unique_ptr<Callback::ManagementServer> managementService;
    std::unique_ptr<Callback::TraceServer> traceServer;
    std::shared_ptr<App::Common::BaseThread> channelThread;
    std::unique_ptr<App::Prometheus::PrometheusExposer>& exposer_;
};
} // namespace App::Source::SkyWalking::Grpc