#include "app/include/intercept/opentelemetry/trace/skywalking/processor.h"
#include "app/include/intercept/opentelemetry/trace/skywalking/skyresource.h"
#include "app/include/intercept/opentelemetry/trace/skywalking/skywalkingproto_to_traces_util.h"
#include <opentelemetry-proto/opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>
#include <spdlog/spdlog.h>

using namespace App::Intercept::Opentelemetry::Trace::Skywalking;
using namespace App::Common::Opentelemetry;

Core::Component::Result Processor::intercept(std::unique_ptr<Core::Component::Batch>& batch) {
    SPDLOG_DEBUG("Entering Processor::intercept with batch size: {}", batch->events().size());
    intercept(*batch);
    return {Core::Component::SUCCESS, ""};
}

Core::Component::Result Processor::intercept(Core::Component::Batch& batch) {
    SPDLOG_DEBUG("Entering Processor::intercept with batch size: {}", batch.events().size());
    auto& events = batch.events();
    if (events.empty()) {
        return {Core::Component::SUCCESS, ""};
    }
    std::vector<std::unique_ptr<Core::Component::EventData>> agg_list;
    for (size_t i = 0; i < events.size(); i++) {
        auto list = process(events[i], i);
        agg_list.insert(agg_list.end(), std::make_move_iterator(list.begin()), std::make_move_iterator(list.end()));
    }
    batch.fill(batch.meta(), agg_list);
    return {Core::Component::SUCCESS, ""};
}

std::vector<std::unique_ptr<OtlpEventData>> Processor::process(std::unique_ptr<Core::Component::EventData>& event,
                                                               long index) {
    counter_.Increment();
    auto *object = (skywalking::v3::SegmentObject*)event->data();
    if (object == nullptr || object->spans().empty()) {
        return {};
    }
    std::vector<std::unique_ptr<OtlpEventData>> recordable_array(object->spans().size());
    // skywalking trace转化工具
    // SkywalkingProtoToTracesUtil util;

    // trace_id
    auto trace_id_span_ = std::array<uint8_t, 16>();
    nostd::span<uint8_t, 16> trace_id(trace_id_span_);
    SPDLOG_INFO("Entering Processor::process for event index: {}, traceid: {}", index, object->traceid());
    auto tmp = SkywalkingProtoToTracesUtil::swTraceIdToTraceId(object->traceid(), trace_id);
    if (tmp.data() == nullptr) {
        SPDLOG_ERROR("invalid trace id: {}", object->traceid());
        return {};
    }
    trace::TraceId traceId(tmp);
    sdk::common::AttributeMap resourceAttr;

    for (const auto& span : object->spans()) {
        swTagsToInternalResource(span, resourceAttr);
    }

    resourceAttr.SetAttribute(AttributeServiceName, object->service());
    resourceAttr.SetAttribute(AttributeServiceInstanceID, object->serviceinstance());
    resourceAttr.SetAttribute(AttributeSkywalkingTraceID, object->traceid());
    auto metadata = event->meta();
    std::string authentication;
    if (metadata.find("authentication") != metadata.end()) {
        authentication = metadata["authentication"];
        resourceAttr.SetAttribute("token", authentication);
        SPDLOG_INFO("set attribute token: {}", authentication);
    }
    auto resource = new SkyResource(resourceAttr, "");

    int count = 0;
    for (auto& span : object->spans()) {
        std::unique_ptr<SkyRecordable> recordable = std::make_unique<SkyRecordable>();

        if (count == 0) {
            recordable->flagFree();
        }

        trace::TraceFlags flags;
        // span_id
        auto span_id_ = std::array<uint8_t, 8>();
        nostd::span<uint8_t, 8> span_id(span_id_);
        SPDLOG_INFO("tracesegmentid: {}, span.spanid: {}", object->tracesegmentid(), span.spanid());
        trace::SpanId spanId(
            SkywalkingProtoToTracesUtil::segmentIDToSpanID(object->tracesegmentid(), span.spanid(), span_id));
        trace::SpanContext context(traceId, spanId, flags, false);

        // parent_span_id
        auto parent_span_id_ = std::array<uint8_t, 8>();
        nostd::span<uint8_t, 8> parent_span_id(parent_span_id_);
        SPDLOG_INFO("refs size: {}", span.refs_size());
        // SPDLOG_INFO("span detail: {}", span.ShortDebugString());
        std::unique_ptr<trace::SpanId> parentSpanIdPtr;
        if (span.parentspanid() != -1) {
            parentSpanIdPtr = std::make_unique<trace::SpanId>(SkywalkingProtoToTracesUtil::segmentIDToSpanID(
                object->tracesegmentid(), span.parentspanid(), parent_span_id));
            recordable->SetIdentity(context, *parentSpanIdPtr);
        } else if (span.refs().size() == 1) {
            auto item = span.refs()[0];
            parentSpanIdPtr = std::make_unique<trace::SpanId>(SkywalkingProtoToTracesUtil::segmentIDToSpanID(
                item.parenttracesegmentid(), item.parentspanid(), parent_span_id));
            recordable->SetIdentity(context, *parentSpanIdPtr);
        } else {
            parentSpanIdPtr = std::make_unique<trace::SpanId>();
            recordable->SetIdentity(context, *parentSpanIdPtr);
        }

        recordable->SetName(span.operationname());
        recordable->SetInstrumentationScope(*Common::Opentelemetry::instrumentationScope);
        common::SystemTimestamp timestamp(
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(span.starttime())));
        recordable->SetStartTime(timestamp);

        recordable->SetDuration(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::milliseconds(span.endtime() - span.starttime())));

        for (int i = 0; i < span.tags().size(); i++) {
            auto& tag = span.tags()[i];
            recordable->SetAttribute(tag.key(), tag.value());
        }
        recordable->SetResource(resource);
        recordable->SetAttribute(AttributeSkywalkingSegmentID, object->tracesegmentid());
        recordable->SetAttribute(AttributeSkywalkingSpanID, span.spanid());
        recordable->SetAttribute(AttributeSkywalkingParentSpanID, span.parentspanid());
        recordable->SetAttribute(AttributePeer, span.peer());
        recordable->SetAttribute(AttributeComponentId, span.componentid());
        if (!authentication.empty()) {
            recordable->SetAttribute("token", authentication);
        }

        setInternalSpanStatus(&span, recordable);

        swLogsToSpanEvents(&span, recordable);

        swSpanKindToSpanKind(&span, recordable);

        if (!span.refs().empty()) {
            swReferencesToSpanLinks(&span, recordable);
        } else {
            trace::SpanContext linkContext(traceId, *parentSpanIdPtr, flags, false);
            std::map<std::string, common::AttributeValue> linkAttr = {{
                AttributeRefType,
                (int)trace::SpanKind::kInternal,
            }};
            recordable->AddLink(linkContext,
                                common::KeyValueIterableView<std::map<std::string, common::AttributeValue>>(linkAttr));
        }
        std::unique_ptr<Core::Component::Data> data =
            std::unique_ptr<Core::Component::Data>(static_cast<Core::Component::Data*>(recordable.release()));
        recordable_array[count] = std::move(std::make_unique<OtlpEventData>(std::move(data)));
        count++;
    }
    return recordable_array;
}

void Processor::setInternalSpanStatus(const skywalking::v3::SpanObject* span,
                                      std::unique_ptr<SkyRecordable>& recordable) {
    if (span->iserror()) {
        recordable->SetStatus(trace::StatusCode::kError, "ERROR");
    } else {
        recordable->SetStatus(trace::StatusCode::kOk, "SUCCESS");
    }
}

void Processor::swTagsToInternalResource(const skywalking::v3::SpanObject& span, sdk::common::AttributeMap& attr) {
    if (span.tags().empty()) {
        return;
    }

    for (auto& tag : span.tags()) {
        auto iter = otSpanTagsMapping.find(tag.key());
        if (iter != otSpanTagsMapping.end()) {
            attr.SetAttribute(iter->second, tag.value());
        }
    }
}

void Processor::swLogsToSpanEvents(const skywalking::v3::SpanObject* span, std::unique_ptr<SkyRecordable>& recordable) {
    if (span->logs().empty()) {
        return;
    }
    std::map<std::string, std::string> attributes;
    for (auto& event : span->logs()) {
        for (auto& d : event.data()) {
            attributes[d.key()] = d.value();
        }
    }

    recordable->AddEvent("logs", std::chrono::system_clock::now(),
                         common::KeyValueIterableView<std::map<std::string, std::string>>(attributes));
}

void Processor::swSpanKindToSpanKind(const skywalking::v3::SpanObject* span,
                                     std::unique_ptr<SkyRecordable>& recordable) {
    if (span->spanlayer() == skywalking::v3::SpanLayer::MQ) {
        if (span->spantype() == skywalking::v3::SpanType::Entry) {
            recordable->SetSpanKind(trace::SpanKind::kConsumer);
            return;
        } else if (span->spantype() == skywalking::v3::SpanType::Exit) {
            recordable->SetSpanKind(trace::SpanKind::kProducer);
            return;
        }
    }

    switch (span->spantype()) {
    case skywalking::v3::SpanType::Entry:
        recordable->SetSpanKind(trace::SpanKind::kServer);
        break;
    case skywalking::v3::SpanType::Exit:
        recordable->SetSpanKind(trace::SpanKind::kClient);
        break;
    case skywalking::v3::SpanType::Local:
        recordable->SetSpanKind(trace::SpanKind::kInternal);
        break;
    default:
        recordable->SetSpanKind(trace::SpanKind::kInternal);
        break;
    }
    return;
}

void Processor::swReferencesToSpanLinks(const skywalking::v3::SpanObject* span,
                                        std::unique_ptr<SkyRecordable>& recordable) {
    if (!span) {
        return;
    }
    if (span->refs().empty()) {
        return;
    }

    for (auto& ref : span->refs()) {
        // trace_id
        auto trace_id_span_ = std::array<uint8_t, 16>();
        nostd::span<uint8_t, 16> trace_id(trace_id_span_);
        trace::TraceId traceId(SkywalkingProtoToTracesUtil::swTraceIdToTraceId(ref.traceid(), trace_id));

        // span_id
        auto parentspanid_ = std::array<uint8_t, 8>();
        nostd::span<uint8_t, 8> parentspanid(parentspanid_);
        trace::SpanId spanId(SkywalkingProtoToTracesUtil::segmentIDToSpanID(ref.parenttracesegmentid(),
                                                                            ref.parentspanid(), parentspanid));
        trace::TraceFlags flags;
        std::map<std::string, common::AttributeValue> attr = {
            {
                AttributeParentService,
                ref.parentservice(),
            },
            {
                AttributeParentInstance,
                ref.parentserviceinstance(),
            },
            {
                AttributeParentEndpoint,
                ref.parentspanid(),
            },
            {
                AttributeNetworkAddressUsedAtPeer,
                ref.networkaddressusedatpeer(),
            },
            {
                AttributeRefType,
                ref.reftype(),
            },
            {
                AttributeSkywalkingTraceID,
                ref.traceid(),
            },
            {AttributeSkywalkingParentSegmentID, ref.parenttracesegmentid()},
            {AttributeSkywalkingParentSpanID, ref.parentspanid()},
        };
        trace::SpanContext context(traceId, spanId, flags, false);
        recordable->AddLink(context, common::KeyValueIterableView<std::map<std::string, common::AttributeValue>>(attr));
    }
}
