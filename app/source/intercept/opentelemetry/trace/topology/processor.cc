#include "app/include/intercept/opentelemetry/trace/topology/processor.h"

#include <opentelemetry-proto/opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>

#include "app/include/common/opentelemetry/otlp_recordable_utils.h"
#include "app/include/common/opentelemetry/recordable.h"
#include "app/include/intercept/opentelemetry/trace/topology/rpc_analysis_listener.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {

Processor::Processor() {
    listenerManager = std::make_unique<ListenerManager>();
    listenerManager->add(std::make_unique<RPCAnalysisListener>());
};
Core::Component::Result Processor::intercept(std::unique_ptr<Core::Component::Batch> &batch) {

}

Core::Component::Result Processor::intercept(Core::Component::Batch &batch) {
    auto& events = batch.events();
    if (events.empty()) {
        return {Core::Component::SUCCESS, ""};
    }
    std::vector<std::unique_ptr<Core::Component::EventData>> agg_list;
    for (size_t i = 0; i < events.size(); i++) {
        auto& e = batch.events()[i];
        auto recordable = (App::Common::Opentelemetry::Recordable*)(std::move(e.get()->dataPtr()).get());

        if (!recordable) {
            continue;
        }

        if (recordable->span().kind() == opentelemetry::proto::trace::v1::Span_SpanKind::Span_SpanKind_SPAN_KIND_SERVER) {
            for (auto& listener : listenerManager->list()) {
                listener->parseEntry(recordable->span(), recordable);
            }
        } else if (recordable->span().kind() == opentelemetry::proto::trace::v1::Span_SpanKind::Span_SpanKind_SPAN_KIND_CLIENT) {
            for (auto& listener : listenerManager->list()) {
                listener->parseExit(recordable->span(), recordable);
            }
        }  else {
            for (auto& listener : listenerManager->list()) {
                listener->parseLocal(recordable->span(), recordable);
            }
        }
    }
}


}
