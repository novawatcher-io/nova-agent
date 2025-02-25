//
// Created by zhanglei on 2025/2/16.
//

#pragma once

#include "listener.h"
#include "endpoint_dependency_builder.h"
#include "app/include/sink/topology/sink.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
class EndpointDepFromCrossThreadAnalysisListener :public Listener {
public:
    EndpointDepFromCrossThreadAnalysisListener(bool enabled_) :enabled(enabled_) {}

    void parseEntry(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                    const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void parseExit(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                   const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void parseLocal(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                    const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void build() final;

    virtual ~EndpointDepFromCrossThreadAnalysisListener() {}
private:

    void parseRefForEndpointDependency(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
    const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) ;

    bool enabled = false;

    std::list<std::unique_ptr<EndpointDependencyBuilder>> depBuilders;
    std::unique_ptr<Sink::Topology::Sink> sink;
};
}