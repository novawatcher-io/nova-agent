#include "app/include/intercept/opentelemetry/trace/topology/processor.h"

#include <chrono>

#include <opentelemetry-proto/opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>
#include <opentelemetry/sdk/common/attribute_utils.h>

#include "app/include/common/opentelemetry/otlp_recordable_utils.h"
#include "app/include/common/opentelemetry/recordable.h"
#include "app/include/intercept/opentelemetry/trace/topology/rpc_analysis_listener.h"
#include "app/include/intercept/opentelemetry/trace/topology/endpoint_dep_from_cross_thread_analysis_listener.h"
#include "app/include/intercept/opentelemetry/trace/topology/network_address_alias_mapping_listener.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {

using namespace opentelemetry::proto::trace::v1;
using namespace Common::Trace::Skywalking;
using namespace Common::Trace::Topology;
using namespace opentelemetry;


// span attr to map
std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue> attrToMap(const opentelemetry::proto::trace::v1::Span &span) {
    std::unordered_map <std::string, ::opentelemetry::proto::common::v1::AnyValue> attrs;
    for (auto iter = span.attributes().begin(); iter != span.attributes().end(); iter++) {
        attrs[iter->key()] = iter->value();
    }
    return attrs;
}

void loadResource(std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr, const opentelemetry::sdk::resource::Resource* resource) {
    if (resource->GetAttributes().empty()) {
        return;
    }
    auto resourceAttr = resource->GetAttributes().GetAttributes();
    for (auto iter = resourceAttr.begin(); iter != resourceAttr.end(); iter++) {
        ::opentelemetry::proto::common::v1::AnyValue proto_value;
        auto value = iter->second;
        if (nostd::holds_alternative<bool>(value))
        {
            proto_value.set_bool_value(nostd::get<bool>(value));
        }
        else if (nostd::holds_alternative<int32_t>(value))
        {
            proto_value.set_int_value(nostd::get<int32_t>(value));
        }
        else if (nostd::holds_alternative<int64_t>(value))
        {
            proto_value.set_int_value(nostd::get<int64_t>(value));
        }
        else if (nostd::holds_alternative<uint32_t>(value))
        {
            proto_value.set_int_value(nostd::get<uint32_t>(value));
        }
        else if (nostd::holds_alternative<uint64_t>(value))
        {
            proto_value.set_int_value(nostd::get<uint64_t>(value));
        }
        else if (nostd::holds_alternative<double>(value))
        {
            proto_value.set_double_value(nostd::get<double>(value));
        }
        else if (nostd::holds_alternative<std::string>(value))
        {
            proto_value.set_string_value(nostd::get<std::string>(value));
        }
        else if (nostd::holds_alternative<std::vector<bool>>(value))
        {
            auto array_value = proto_value.mutable_array_value();
            for (const auto val : nostd::get<std::vector<bool>>(value))
            {
                array_value->add_values()->set_bool_value(val);
            }
        }
        else if (nostd::holds_alternative<std::vector<int32_t>>(value))
        {
            auto array_value = proto_value.mutable_array_value();
            for (const auto &val : nostd::get<std::vector<int32_t>>(value))
            {
                array_value->add_values()->set_int_value(val);
            }
        }
        else if (nostd::holds_alternative<std::vector<uint32_t>>(value))
        {
            auto array_value = proto_value.mutable_array_value();
            for (const auto &val : nostd::get<std::vector<uint32_t>>(value))
            {
                array_value->add_values()->set_int_value(val);
            }
        }
        else if (nostd::holds_alternative<std::vector<int64_t>>(value))
        {
            auto array_value = proto_value.mutable_array_value();
            for (const auto &val : nostd::get<std::vector<int64_t>>(value))
            {
                array_value->add_values()->set_int_value(val);
            }
        }
        else if (nostd::holds_alternative<std::vector<uint64_t>>(value))
        {
            auto array_value = proto_value.mutable_array_value();
            for (const auto &val : nostd::get<std::vector<uint64_t>>(value))
            {
                array_value->add_values()->set_int_value(val);
            }
        }
        else if (nostd::holds_alternative<std::vector<double>>(value))
        {
            auto array_value = proto_value.mutable_array_value();
            for (const auto &val : nostd::get<std::vector<double>>(value))
            {
                array_value->add_values()->set_double_value(val);
            }
        }
        else if (nostd::holds_alternative<std::vector<std::string>>(value))
        {
            auto array_value = proto_value.mutable_array_value();
            for (const auto &val : nostd::get<std::vector<std::string>>(value))
            {
                array_value->add_values()->set_string_value(val);
            }
        }
        attr[iter->first] = proto_value;
    }
    return;
}


Processor::Processor(std::shared_ptr<App::Config::ConfigReader> config_,
 const std::shared_ptr<Core::Event::EventLoop>& loop_) :loop(loop_) {
    Common::Grpc::ClientOptions options;
    options.max_concurrent_requests = 1000;
    options.endpoint = config_->NodeReportHost();
    options.metadata.insert({"company_uuid", config_->GetConfig().company_uuid()});
    sink = std::make_unique<Sink::Topology::Sink>(options);
    listenerManager = std::make_unique<ListenerManager>();
    listenerManager->add(std::make_unique<RPCAnalysisListener>(true, sink));
//    listenerManager->add(std::make_unique<EndpointDepFromCrossThreadAnalysisListener>(true));
//    listenerManager->add(std::make_unique<NetworkAddressAliasMappingListener>(false));
};
Core::Component::Result Processor::intercept(std::unique_ptr<Core::Component::Batch> &batch) {
    intercept(*batch);
    return {Core::Component::SUCCESS, ""};
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

        auto spanAttr = attrToMap(recordable->span());
        loadResource(spanAttr, recordable->GetResource());
        if (recordable->span().kind() == opentelemetry::proto::trace::v1::Span_SpanKind::Span_SpanKind_SPAN_KIND_SERVER) {
            for (auto& listener : listenerManager->list()) {
                listener->parseEntry(recordable->span(), recordable, spanAttr);
            }
        } else if (recordable->span().kind() == opentelemetry::proto::trace::v1::Span_SpanKind::Span_SpanKind_SPAN_KIND_CLIENT) {
            for (auto& listener : listenerManager->list()) {
                listener->parseExit(recordable->span(), recordable, spanAttr);
            }
        }  else {
            for (auto& listener : listenerManager->list()) {
                listener->parseLocal(recordable->span(), recordable, spanAttr);
            }
        }
    }

    for (auto& listener : listenerManager->list()) {
        listener->build();
    }
    return {Core::Component::SUCCESS, ""};
}

void Processor::flushMetric() {
    SPDLOG_DEBUG("flushMetric");
    for (auto& listener : listenerManager->list()) {
        listener->flush();
    }
}

void Processor::clearTopologyMetric() {
    SPDLOG_DEBUG("clearTopologyMetric");
    for (auto& listener : listenerManager->list()) {
        listener->clear();
    }
}

void Processor::start() {
    timer_ = std::make_shared<Core::Component::TimerChannel>(loop, [this]() {
        flushMetric();
        timer_->enable(std::chrono::seconds(60));
    });
    clearMetricTimer_ = std::make_shared<Core::Component::TimerChannel>(loop, [this]() {
        clearTopologyMetric();
        clearMetricTimer_->enable(std::chrono::seconds(60));
    });
//    timer_->enable(std::chrono::minutes(5));
    timer_->enable(std::chrono::seconds(60));
    clearMetricTimer_->enable(std::chrono::seconds(300));
}

void Processor::stop() {
    timer_->disable();
    clearMetricTimer_->disable();
}
}
