//
// Created by zhanglei on 2025/2/16.
//
#pragma once
#include <opentelemetry-proto/opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>

#include "pure.h"

#include <app/include/common/opentelemetry/recordable.h>

namespace App::Intercept::Opentelemetry::Trace::Topology {
class Listener {
public:
     virtual void parseEntry(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                             const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) PURE;

     virtual void parseExit(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                            const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) PURE;

     virtual void parseLocal(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                             const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) PURE;

     virtual void build() PURE;

     virtual ~Listener() {
     };
};
}
