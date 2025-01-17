//
// Created by root on 2024/4/19.
//
#include "app/include/sink/opentelemetry/metric/grpc/sink.h"
#include "app/include/common/opentelemetry/otlp_metric_resource.h"
#include "app/include/common/opentelemetry/otlp_metric_utils.h"

using namespace App::Sink::OpenTelemetry::Metric::Grpc;

Core::Component::Result Sink::Consume(Core::Component::Batch& batch) {
    if (batch.events().empty()) {
        return {};
    }

    std::vector<std::unique_ptr<Common::Opentelemetry::OtlpMetricResource>> metric_array(batch.events().size());
    for (size_t i = 0; i < batch.events().size(); i++) {
        auto& e = batch.events()[i];
        auto metricRecordable = std::unique_ptr<Common::Opentelemetry::OtlpMetricResource>(
            (Common::Opentelemetry::OtlpMetricResource*)(std::move(e->dataPtr()).release()));
        metric_array[i] = std::move(metricRecordable);
        continue;
    }

    if (metric_array.empty()) {
        return {};
    }

    google::protobuf::ArenaOptions arena_options;
    // It's easy to allocate datas larger than 1024 when we populate basic resource and attributes
    arena_options.initial_block_size = 1024;
    // When in batch mode, it's easy to export a large number of spans at once, we can alloc a lager
    // block to reduce memory fragments.
    arena_options.max_block_size = 65536;
    auto arena = std::make_unique<google::protobuf::Arena>(arena_options);

    ExportMetricsServiceRequest* request = google::protobuf::Arena::Create<ExportMetricsServiceRequest>(arena.get());
    for (auto& metric : metric_array) {
        Common::Opentelemetry::OtlpMetricUtils::PopulateRequest(*metric, request);
    }

    client->Export(metric_service_stub_, std::move(*request), std::move(arena),
                   [](Common::OpenTelemetry::ExportTraceServiceCallData<ExportMetricsServiceResponse>*) -> bool {
                       return true;
                   });

    return {Core::Component::SUCCESS, "success"};
}
