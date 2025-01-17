#include "app/include/common/opentelemetry/otlp_recordable_utils.h"
#include <opentelemetry/sdk/instrumentationscope/instrumentation_scope.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/recordable.h>
#include <cstddef>
#include <unordered_map>
#include <utility>
#include <vector>
#include "app/include/common/opentelemetry/otlp_populate_attribute_utils.h"
#include "app/include/common/opentelemetry/recordable.h"
#include "opentelemetry/proto/collector/trace/v1/trace_service.pb.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/resource/v1/resource.pb.h"
#include "opentelemetry/proto/trace/v1/trace.pb.h"
namespace App {
namespace Common {
namespace Opentelemetry {
namespace {
struct InstrumentationScopePointerHasher {
    std::size_t
    operator()(const opentelemetry::sdk::instrumentationscope::InstrumentationScope* instrumentation) const noexcept {
        return instrumentation->HashCode();
    }
};

struct InstrumentationScopePointerEqual {
    std::size_t operator()(const opentelemetry::sdk::instrumentationscope::InstrumentationScope* left,
                           const opentelemetry::sdk::instrumentationscope::InstrumentationScope* right) const noexcept {
        return *left == *right;
    }
};
} // namespace

void OtlpRecordableUtils::PopulateRequest(
    const nostd::span<std::unique_ptr<opentelemetry::sdk::trace::Recordable>>& spans,
    proto::collector::trace::v1::ExportTraceServiceRequest* request) noexcept {
    if (nullptr == request) {
        return;
    }

    using spans_by_scope = std::unordered_map<const opentelemetry::sdk::instrumentationscope::InstrumentationScope*,
                                              std::vector<std::unique_ptr<Recordable>>>;
    std::unordered_map<const opentelemetry::sdk::resource::Resource*, spans_by_scope> spans_index;

    // Collect spans per resource and instrumentation scope
    for (auto& recordable : spans) {
        auto ptr = (Recordable*)recordable.get();
        auto resource = ptr->GetResource();
        auto instrumentation = ptr->GetInstrumentationScope();
        auto rec = std::unique_ptr<Recordable>(static_cast<Recordable*>(recordable.release()));
        spans_index[resource][instrumentation].emplace_back(std::move(rec));
    }

    // Add all resource spans
    for (auto& input_resource_spans : spans_index) {
        // Add the resource
        auto resource_spans = request->add_resource_spans();
        if (input_resource_spans.first) {
            proto::resource::v1::Resource resource_proto;
            OtlpPopulateAttributeUtils::PopulateAttribute(&resource_proto, *input_resource_spans.first);
            *resource_spans->mutable_resource() = resource_proto;
            resource_spans->set_schema_url(input_resource_spans.first->GetSchemaURL());
        }

        // Add all scope spans
        for (auto& input_scope_spans : input_resource_spans.second) {
            // Add the instrumentation scope
            auto scope_spans = resource_spans->add_scope_spans();
            if (input_scope_spans.first) {
                proto::common::v1::InstrumentationScope instrumentation_scope_proto;
                instrumentation_scope_proto.set_name(input_scope_spans.first->GetName());
                instrumentation_scope_proto.set_version(input_scope_spans.first->GetVersion());
                *scope_spans->mutable_scope() = instrumentation_scope_proto;
                scope_spans->set_schema_url(input_scope_spans.first->GetSchemaURL());
            }

            // Add all spans to this scope spans
            for (auto& input_span : input_scope_spans.second) {
                *scope_spans->add_spans() = std::move(input_span->span());
            }
        }
    }
}
} // namespace Opentelemetry
} // namespace Common
} // namespace App
