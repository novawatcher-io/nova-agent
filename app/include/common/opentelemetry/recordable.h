//
// Created by root on 2024/3/31.
//
#pragma once

#include "component/api.h"
#include "opentelemetry/proto/resource/v1/resource.pb.h"
#include "opentelemetry/proto/trace/v1/trace.pb.h"
#include "opentelemetry/sdk/resource/resource.h"
#include "opentelemetry/sdk/trace/recordable.h"

#include "otlp_recordable_utils.h"

namespace App::Common::Opentelemetry {

class Recordable : public opentelemetry::sdk::trace::Recordable,
                   public Core::Component::Data {
public:
    void* data() override {
        return this;
    }
    proto::trace::v1::Span& span() noexcept {
        return span_;
    }
    const proto::trace::v1::Span& span() const noexcept {
        return span_;
    }

    /** Dynamically converts the resource of this span into a proto. */
    proto::resource::v1::Resource ProtoResource() const noexcept;

    const opentelemetry::sdk::resource::Resource* GetResource() const noexcept;
    const std::string GetResourceSchemaURL() const noexcept;
    const opentelemetry::sdk::instrumentationscope::InstrumentationScope* GetInstrumentationScope() const noexcept;
    const std::string GetInstrumentationLibrarySchemaURL() const noexcept;

    proto::common::v1::InstrumentationScope GetProtoInstrumentationScope() const noexcept;

    void SetIdentity(const opentelemetry::trace::SpanContext& span_context,
                     opentelemetry::trace::SpanId parent_span_id) noexcept override;

    void SetAttribute(opentelemetry::nostd::string_view key,
                      const opentelemetry::common::AttributeValue& value) noexcept override;

    void AddEvent(opentelemetry::nostd::string_view name, opentelemetry::common::SystemTimestamp timestamp,
                  const opentelemetry::common::KeyValueIterable& attributes) noexcept override;

    void AddLink(const opentelemetry::trace::SpanContext& span_context,
                 const opentelemetry::common::KeyValueIterable& attributes) noexcept override;

    void SetStatus(opentelemetry::trace::StatusCode code, nostd::string_view description) noexcept override;

    void SetName(nostd::string_view name) noexcept override;

    void SetTraceFlags(opentelemetry::trace::TraceFlags flags) noexcept override;

    void SetSpanKind(opentelemetry::trace::SpanKind span_kind) noexcept override;

    void SetResource(const opentelemetry::sdk::resource::Resource& resource) noexcept override;

    void SetStartTime(opentelemetry::common::SystemTimestamp start_time) noexcept override;

    void SetDuration(std::chrono::nanoseconds duration) noexcept override;

    void SetInstrumentationScope(
        const opentelemetry::sdk::instrumentationscope::InstrumentationScope& instrumentation_scope) noexcept override;

private:
    proto::trace::v1::Span span_;
    const opentelemetry::sdk::resource::Resource* resource_ = nullptr;
    const opentelemetry::sdk::instrumentationscope::InstrumentationScope* instrumentation_scope_ = nullptr;
};

} // namespace App::Common::Opentelemetry
