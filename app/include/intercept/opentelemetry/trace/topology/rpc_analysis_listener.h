//
// Created by zhanglei on 2025/2/16.
//
#pragma once

#include "listener.h"
#include "rpc_traffic_source_builder.h"
#include "endpoint_source_builder.h"
#include "app/include/common/lrucache.h"
#include "app/include/sink/topology/sink.h"
#include "service_metric_aggregator.h"
#include "service_relation_metric_aggregator.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
class RPCAnalysisListener :public Listener {
public:
    RPCAnalysisListener(bool enabled_, const std::unique_ptr<Sink::Topology::Sink> &sink) :enabled(enabled_), sink_(sink) {
        inLruCache = std::make_unique<App::Common::LruCache<uint64_t, bool>>(500);
        outLruCache = std::make_unique<App::Common::LruCache<uint64_t, bool>>(500);
        serviceMetricAggregator = std::make_unique<ServiceMetricAggregator>(500, sink_);
        serviceRelationMetricAggregator = std::make_unique<ServiceRelationMetricAggregator>(500, sink_);
    }

    void parseEntry(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                    const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void parseExit(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                   const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void parseLocal(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                    const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void build() final;

    void flush() final;

    void clear() final;

    virtual ~RPCAnalysisListener() {
    }

private:
    void parseLogicEndpoints(const opentelemetry::proto::trace::v1::Span& span,
     const App::Common::Opentelemetry::Recordable* recordable,
     const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr);

    std::list<std::unique_ptr<RPCTrafficSourceBuilder>> callingInTraffic;

    std::list<std::unique_ptr<RPCTrafficSourceBuilder>> callingOutTraffic;

    std::list<std::unique_ptr<EndpointSourceBuilder>> logicEndpointBuilders;

    std::unique_ptr<App::Common::LruCache<uint64_t, bool>> inLruCache;

    std::unique_ptr<App::Common::LruCache<uint64_t, bool>> outLruCache;

    bool enabled = false;

    const std::unique_ptr<Sink::Topology::Sink> &sink_;

    std::unique_ptr<ServiceMetricAggregator> serviceMetricAggregator;
    std::unique_ptr<ServiceRelationMetricAggregator> serviceRelationMetricAggregator;
};
}
