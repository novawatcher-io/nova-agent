#pragma once

#include <component/api.h>
#include <map>
#include <memory>
#include <opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.h>
#include <skywalking-data-collect-protocol/logging/Logging.grpc.pb.h>
#include <string>
#include <utility>

namespace App::Source::SkyWalking::Grpc {
using namespace opentelemetry::proto::collector::logs::v1;
using namespace skywalking::v3;

class LoggingServerEventData : public Core::Component::EventData {
public:
    explicit LoggingServerEventData(std::unique_ptr<Core::Component::Data> data) : data_(std::move(data)) {
    }

    std::map<std::string, std::string> meta() override {
        return {};
    }

    std::map<std::string, std::string> header() override {
        return {};
    }

    std::unique_ptr<Core::Component::Data>& dataPtr() override {
        return data_;
    }

    std::string name() override {
        return "TraceSegmentServerEventData";
    }

    void* data() override {
        return data_->data();
    }

    // Fill event with meta,header,body cannot be nil
    void fill(const std::map<std::string, std::string>& meta, const std::map<std::string, std::string>& /*header*/,
              std::unique_ptr<Core::Component::Data> data) override {
        meta_ = meta;
        data_ = std::move(data);
    }

    void addMeta(const std::string& key, const std::string& value) {
        meta_[key] = value;
    }

    void release() override {
        meta_.clear();
        data_.reset();
    }

private:
    std::map<std::string, std::string> meta_;
    std::unique_ptr<Core::Component::Data> data_;
};
} // namespace App::Source::SkyWalking::Grpc
