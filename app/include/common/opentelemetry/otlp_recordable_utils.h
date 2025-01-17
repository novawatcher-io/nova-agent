#pragma once

#include <opentelemetry/nostd/span.h>            // for span
#include <memory>                                // for unique_ptr
#include "opentelemetry/sdk/trace/recordable.h"  // for Recordable
namespace opentelemetry { namespace proto { namespace collector { namespace trace { namespace v1 { class ExportTraceServiceRequest; } } } } }

namespace App {
namespace Common {
namespace Opentelemetry {
using namespace opentelemetry;
/**
 * The OtlpRecordableUtils contains utility functions for OTLP recordable
 */
class OtlpRecordableUtils
{
public:
    static void PopulateRequest(
            const nostd::span<std::unique_ptr<opentelemetry::sdk::trace::Recordable>> &spans,
            proto::collector::trace::v1::ExportTraceServiceRequest *request) noexcept;
};
}
}
}
