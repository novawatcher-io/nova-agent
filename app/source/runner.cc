#include "app/include/runner.h"

#include <opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_options.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/meter_context_factory.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/metrics/view/view_registry_factory.h>
#include <os/unix_countdown_latch.h>
#include <os/unix_thread.h>

#include "app/include/common/machine.h"
#include "app/include/sink/channel/grpc_channel.h"
#include "app/include/source/host/source.h"
#include "app/include/source/skywalking/grpc/source.h"
#include "app/include/source/http/source.h"
#include <component/thread_container.h>
#include <csignal>
#include <spdlog/spdlog.h>

namespace App {
namespace otlp_exporter = opentelemetry::exporter::otlp;
namespace metric_sdk = opentelemetry::sdk::metrics;
namespace metrics_api = opentelemetry::metrics;

static std::string GetHostname() {
    char hostname[256] = {0};
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return hostname;
    }
    return "";
}

void Runner::onstop(evutil_socket_t /*sig*/, short /*events*/, void* param) {
    auto* node = static_cast<Runner*>(param);
    // 主线程执行结束后会依次执行所有模块的shutdown从而停止所有服务
    SPDLOG_DEBUG("stop signal received, stopping...");
    node->loop->quit();
}

void Runner::initMetricExporter() {
    otlp_exporter::OtlpGrpcMetricExporterOptions exporter_options;
    // exporter_options.endpoint = "81.71.98.26:4317";
    exporter_options.endpoint = config_->OLTPExporterAddress();
    SPDLOG_DEBUG("OLTP exporter address: {}", exporter_options.endpoint);
    // 初始化导出器
    auto exporter = otlp_exporter::OtlpGrpcMetricExporterFactory::Create(exporter_options);
    // std::string version{"1.2.0"};
    // std::string schema{"https://opentelemetry.io/schemas/1.2.0"};
    metric_sdk::PeriodicExportingMetricReaderOptions reader_options;
    reader_options.export_interval_millis = std::chrono::milliseconds(1000);
    reader_options.export_timeout_millis = std::chrono::milliseconds(500);
    // Initialize and set the global MeterProvider
    auto reader = metric_sdk::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), reader_options);
    // 创建 ResourceAttributes 对象
    opentelemetry::sdk::resource::ResourceAttributes resource_attributes{
        {"agent_name", "deep-observe-agent"},
        {"service.name", GetHostname()},
        {"telemetry.sdk.version", OPENTELEMETRY_VERSION},
        {"telemetry.sdk.language", "cpp"},
        {"object_id", std::to_string(Common::getMachineId())},
    };
    auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);
    auto views = metric_sdk::ViewRegistryFactory::Create();
    auto context = metric_sdk::MeterContextFactory::Create(std::move(views), resource);
    context->AddMetricReader(std::move(reader));
    auto u_provider = metric_sdk::MeterProviderFactory::Create(std::move(context));
    std::shared_ptr<opentelemetry::metrics::MeterProvider> provider(std::move(u_provider));

    metrics_api::Provider::SetMeterProvider(provider);
}

void Runner::cleanupMetrics() {
    SPDLOG_DEBUG("cleanup metrics");
    std::shared_ptr<metrics_api::MeterProvider> none;
    metrics_api::Provider::SetMeterProvider(none);
}

void Runner::run() {
    // 初始化完成后安装信号处理器
    loop->sigAdd(SIGTERM, onstop, this);

    // 注册模块
    initMetricExporter();
    auto exposer = std::make_unique<App::Prometheus::PrometheusExposer>();


    if (!config_->GetConfig().IsInitialized()) {
        SPDLOG_ERROR("parse config error!container_collector_config is null");
        return;
    }

    App::Source::SkyWalking::Grpc::Source* source = nullptr;
    if (config_->GetConfig().has_trace_server_config() && config_->GetConfig().trace_server_config().enable()) {
        // 调用链stream
        // 启动source
        source = new App::Source::SkyWalking::Grpc::Source(config_, exposer);
        sourceThread->addInitCallable([&] {
            source->start();
            delete source;
        });
        sourceThread->start();
    }

    std::unique_ptr<Source::Host::Source> hostSource;
    auto threadManager = std::make_shared<Core::Component::UnixThreadContainer>();
    // 初始化线程池
    for (int i = 0; i < 2; i++) {
        auto thread = std::make_unique<Core::OS::UnixThread>();
        threadManager->reg(i, thread);
    }

    // 启动http模块
    auto httpSource =
            std::make_unique<Source::Http::Source>(loop, threadManager, config_, exposer);
    // 启动http模块
    if (config_->GetConfig().has_http_server_config() && config_->GetConfig().http_server_config().enable()) {
        httpSource->init();
        httpSource->start();
    }

    threadManager->start();


    if (config_->GetConfig().has_host_collect_config() && config_->GetConfig().host_collect_config().enable()) {
        hostSource =
            std::make_unique<Source::Host::Source>(loop, threadManager, config_);
        hostSource->init();
        hostSource->start();
    }


    loop->loop();
    SPDLOG_INFO("main loop stopped, start to shutdown...");
    //    manager->shutdown();
    if (config_->GetConfig().has_trace_server_config() && config_->GetConfig().trace_server_config().enable()) {
        SPDLOG_INFO("trace server start to shutdown...");
        source->stop();
        SPDLOG_DEBUG("source thread start to shutdown...");
        sourceThread->stop();
        SPDLOG_DEBUG("source thread stopped");
        grpcChannelThread->stop();
        SPDLOG_DEBUG("grpc channel thread stopped");
    }

    if (config_->GetConfig().has_host_collect_config() && config_->GetConfig().host_collect_config().enable()) {
        if (hostSource) {
            hostSource->stop();
            hostSource->finish();
        }
    }

    if (config_->GetConfig().has_http_server_config() && config_->GetConfig().http_server_config().enable()) {
        httpSource->stop();
    }
    cleanupMetrics();
    threadManager->stop();
    SPDLOG_INFO("app stopped");
}
} // namespace App
