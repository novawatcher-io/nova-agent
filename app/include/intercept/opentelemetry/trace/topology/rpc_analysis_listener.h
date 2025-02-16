//
// Created by zhanglei on 2025/2/16.
//
#pragma once

#include "listener.h"
#include "rpc_traffic_source_builder.h"
#include "endpoint_source_builder.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
class RPCAnalysisListener :public Listener {
public:
    void parseEntry(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable) final;

    void parseExit(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable) final;

    void parseLocal(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable) final;

private:
    std::list<std::unique_ptr<RPCTrafficSourceBuilder>> callingInTraffic;

    std::list<std::unique_ptr<RPCTrafficSourceBuilder>> callingOutTraffic;

    std::list<std::unique_ptr<EndpointSourceBuilder>> logicEndpointBuilders;
};
}
