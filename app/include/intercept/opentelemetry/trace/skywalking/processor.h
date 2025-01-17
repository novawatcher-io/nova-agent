#pragma once
#include "app/include/common/opentelemetry/otlp_event_data.h"
#include "app/include/prometheus/prometheus_exposer.h"
#include "const.h"
#include "opentelemetry/sdk/trace/recordable.h"
#include "skyrecordable.h"
#include "skywalking-data-collect-protocol/language-agent/Tracing.grpc.pb.h"
#include <prometheus/counter.h>

using namespace App::Common::Opentelemetry;

namespace App::Intercept::Opentelemetry::Trace::Skywalking {
class Processor : public Core::Component::Interceptor {
public:
    Processor(App::Prometheus::PrometheusExposer& exposer)
        : counter_(exposer.AddCounter("trace", "skywalking trace metric", {{"trace", "qps"}})) {};

    ~Processor() = default;

    Core::Component::Result intercept(std::unique_ptr<Core::Component::Batch>& batch) override;

    Core::Component::Result intercept(Core::Component::Batch& batch) override;

private:
    std::vector<std::unique_ptr<OtlpEventData>> process(std::unique_ptr<Core::Component::EventData>& event, long index);

    void setInternalSpanStatus(const skywalking::v3::SpanObject* span, std::unique_ptr<SkyRecordable>& recordable);

    void swLogsToSpanEvents(const skywalking::v3::SpanObject* span, std::unique_ptr<SkyRecordable>& recordable);

    void swSpanKindToSpanKind(const skywalking::v3::SpanObject* span, std::unique_ptr<SkyRecordable>& recordable);

    void swReferencesToSpanLinks(const skywalking::v3::SpanObject* span, std::unique_ptr<SkyRecordable>& recordable);

    void swTagsToInternalResource(const skywalking::v3::SpanObject& span, sdk::common::AttributeMap& attr);

    prometheus::Counter& counter_;
};
} // namespace App::Intercept::Opentelemetry::Trace::Skywalking
