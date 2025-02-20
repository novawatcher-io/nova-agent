//
// Created by zhanglei on 2025/2/16.
//

#pragma once
#include "listener.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
class NetworkAddressAliasMappingListener :public Listener {
public:
    NetworkAddressAliasMappingListener(bool enabled_) :enabled(enabled_) {}

    void parseEntry(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                    const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void parseExit(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                   const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void parseLocal(const opentelemetry::proto::trace::v1::Span& span, const App::Common::Opentelemetry::Recordable* recordable,
                    const std::unordered_map<std::string, ::opentelemetry::proto::common::v1::AnyValue>& attr) final;

    void build() final;
private:
    bool enabled = false;
};
}
