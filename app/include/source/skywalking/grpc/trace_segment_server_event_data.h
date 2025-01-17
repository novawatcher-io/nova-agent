#pragma once

#include <component/api.h>

#include <opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>
#include <skywalking-data-collect-protocol/language-agent/Tracing.grpc.pb.h>

namespace App::Source::SkyWalking::Grpc {
using namespace opentelemetry::proto::collector::trace::v1;
using namespace skywalking::v3;

class TraceSegmentServerEventData : public Core::Component::EventData {
public:
    explicit TraceSegmentServerEventData(std::unique_ptr<Core::Component::Data> data) : data_(std::move(data)) {
    }

    std::map<std::string, std::string> meta() override {
        return meta_;
    }

    std::map<std::string, std::string> header() override {
        return {};
    }

    std::unique_ptr<Core::Component::Data>& dataPtr() {
        return data_;
    }

    std::string chainName() {
        return "TraceSegmentServerEventData";
    }

    std::string name() override {
        return "SkyWalkingTraceSegmentServerEventData";
    }

    void* data() override {
        return data_ ? data_->data() : nullptr;
    }

    // Fill event with meta,header,body cannot be nil
    void fill(const std::map<std::string, std::string>&, const std::map<std::string, std::string>&,
              std::unique_ptr<Core::Component::Data> data) override {
        data_ = std::move(data);
    }

    void addMeta(const std::string& key, const std::string& value) {
        meta_[key] = value;
    }

    void release() override {
    }

    ~TraceSegmentServerEventData() = default;

private:
    std::map<std::string, std::string> meta_;
    std::unique_ptr<Core::Component::Data> data_;
};
} // namespace App::Source::SkyWalking::Grpc
