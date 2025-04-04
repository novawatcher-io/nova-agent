#include "app/include/source/skywalking/grpc/source.h"
#include <grpcpp/security/server_credentials.h>
#include <os/unix_countdown_latch.h>
#include <cstdlib>
#include <functional>
#include <utility>
#include "app/include/intercept/opentelemetry/log/skywalking/processor.h"
#include "app/include/intercept/opentelemetry/metric/skywalking/processor.h"
#include "app/include/intercept/opentelemetry/trace/skywalking/processor.h"
#include "app/include/intercept/opentelemetry/trace/topology/processor.h"
#include "app/include/sink/channel/grpc_channel.h"
#include "app/include/sink/opentelemetry/log/grpc/sink.h"
#include "app/include/sink/opentelemetry/metric/grpc/sink.h"
#include "app/include/sink/opentelemetry/trace/grpc/sink.h"
#include "app/include/sink/pipeline/pipeline.h"
#include "app/include/source/skywalking/grpc/callback/trace_server.h"
#include "common/base_thread.h"
#include "component/api.h"
#include "source/skywalking/grpc/callback/configuration_discovery_server.h"
#include "source/skywalking/grpc/callback/jvm_metric_report_server.h"
#include "source/skywalking/grpc/callback/log_report_server.h"
#include "source/skywalking/grpc/callback/management_server.h"
#include "source/skywalking/grpc/callback/meter_report_server.h"
#include "source/skywalking/grpc/callback/clr_metric_server.h"

using grpc::ServerBuilder;
using grpc::Server;
using namespace App::Source::SkyWalking::Grpc;

void Source::init() {
    const char* host = getenv("SOURCE_HOST");
    if (host == nullptr) {
        host = "0.0.0.0:11800";
    }

    std::string address(host);

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    cq_ = builder.AddCompletionQueue();

    std::unique_ptr<Core::Component::Pipeline> tracePipeline;
    // 构建pipeline
    tracePipeline = std::make_unique<App::Sink::Pipeline::Pipeline>();
    tracePipelinePtr = tracePipeline.get();
    std::unique_ptr<Core::Component::Interceptor> traceProcess = std::make_unique<Intercept::Opentelemetry::Trace::Skywalking::Processor>(*exposer_);
    tracePipeline->addIntercept(traceProcess);
    // 构建服务拓扑
    std::unique_ptr<Core::Component::Interceptor> topoProcess = std::make_unique<Intercept::Opentelemetry::Trace::Topology::Processor>(config_, loop);
    tracePipeline->addIntercept(topoProcess);
    std::unique_ptr<Core::Component::Consumer> traceSink = std::make_unique<Sink::OpenTelemetry::Trace::Grpc::Sink>(cq_);
    tracePipeline->addConsumer(traceSink);

    metricPipeline = std::make_unique<App::Sink::Pipeline::Pipeline>();
    std::unique_ptr<Core::Component::Interceptor> metricProcess = std::make_unique<Intercept::Opentelemetry::Metric::Skywalking::Processor>();
    metricPipeline->addIntercept(metricProcess);
    std::unique_ptr<Core::Component::Consumer> metricSink = std::make_unique<Sink::OpenTelemetry::Metric::Grpc::Sink>(cq_);
    metricPipeline->addConsumer(metricSink);

    logPipeline = std::make_unique<App::Sink::Pipeline::Pipeline>();
    std::unique_ptr<Core::Component::Interceptor> logProcess = std::make_unique<Intercept::Opentelemetry::Log::Skywalking::Processor>();
    logPipeline->addIntercept(logProcess);
    std::unique_ptr<Core::Component::Consumer> logSink = std::make_unique<Sink::OpenTelemetry::Log::Grpc::Sink>(cq_);
    logPipeline->addConsumer(logSink);

    traceServer = std::make_unique<Callback::TraceServer>(std::move(tracePipeline));
    configurationDiscoveryService = std::make_unique<Callback::ConfigurationDiscoveryServer>();
    jvmMetricReportService = std::make_unique<Callback::JvmMetricReportServer>(std::move(metricPipeline));
    logReportService = std::make_unique<Callback::LogReportServer>(std::move(logPipeline));
    meterReportService = std::make_unique<Callback::MeterReportServer>();
    managementService = std::make_unique<Callback::ManagementServer>();
    clrMetricServer = std::make_unique<Callback::CLRMetricServer>();
    builder.RegisterService(traceServer.get());
    builder.RegisterService(configurationDiscoveryService.get());
    builder.RegisterService(jvmMetricReportService.get());
    builder.RegisterService(logReportService.get());
    builder.RegisterService(meterReportService.get());
    builder.RegisterService(managementService.get());
    builder.RegisterService(clrMetricServer.get());


    server_ = builder.BuildAndStart();
}

void Source::start() {
    tracePipelinePtr->start();
    server_->Wait();
    SPDLOG_INFO("server finish shutdown...");
}

void Source::stop() {
    SPDLOG_INFO("trace pipeline start to shutdown...");
    if (tracePipelinePtr != nullptr) {
        tracePipelinePtr->stop();
    }
    SPDLOG_INFO("source server start to shutdown...");
    if (server_) {
        server_->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds(5));
    }
    SPDLOG_INFO("channel start to shutdown...");
//    channel->stop();
//    channelThread->stop();
}

void Source::finish(){
}
