#include "app/include/sink/opentelemetry/trace/grpc/sink.h"
#include "app/include/common/opentelemetry/otlp_recordable_utils.h"
#include "app/include/common/opentelemetry/recordable.h"
#include <array>
#include <component/api.h>
#include <spdlog/spdlog.h>

using namespace App::Sink::OpenTelemetry::Trace::Grpc;
using namespace opentelemetry;
using namespace opentelemetry::proto::collector::trace::v1;

Core::Component::Result Sink::Consume(Core::Component::Batch& batch) {
    if (batch.events().empty()) {
        return {};
    }

    std::vector<std::unique_ptr<opentelemetry::sdk::trace::Recordable>> span_array(batch.events().size());
    for (size_t i = 0; i < batch.events().size(); i++) {
        auto& e = batch.events()[i];
        auto recordable = std::unique_ptr<opentelemetry::sdk::trace::Recordable>(
            (App::Common::Opentelemetry::Recordable*)(std::move(e.get()->dataPtr()).release()));
        span_array[i] = std::move(recordable);
    }

    if (span_array.empty()) {
        return {};
    }

    nostd::span<std::unique_ptr<opentelemetry::sdk::trace::Recordable>> spans(span_array.data(), span_array.size());

    google::protobuf::ArenaOptions arena_options;
    // It's easy to allocate datas larger than 1024 when we populate basic resource and attributes
    arena_options.initial_block_size = 1024;
    // When in batch mode, it's easy to export a large number of spans at once, we can alloc a lager
    // block to reduce memory fragments.
    arena_options.max_block_size = 65536;
    auto arena = std::make_unique<google::protobuf::Arena>(arena_options);

    ExportTraceServiceRequest* request = google::protobuf::Arena::Create<ExportTraceServiceRequest>(arena.get());
    Common::Opentelemetry::OtlpRecordableUtils::PopulateRequest(spans, request);
    SPDLOG_INFO("sink request: {}", request->ShortDebugString());

    client->Export(trace_service_stub_, std::move(*request), std::move(arena),
                   [](Common::OpenTelemetry::ExportTraceServiceCallData<ExportTraceServiceResponse>*) -> bool {
                       return true;
                   });

    return {Core::Component::SUCCESS,""};
}
