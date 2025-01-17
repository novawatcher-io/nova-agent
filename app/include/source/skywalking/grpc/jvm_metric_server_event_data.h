#pragma once

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
class JvmMetricServerEventData : public Core::Component::EventData {
public:
    explicit JvmMetricServerEventData(std::unique_ptr<Core::Component::Data> data) : data_(std::move(data)) {
    }

    std::map<std::string, std::string> meta() override {
        return {};
    }

    std::map<std::string, std::string> header() override {
        return {};
    }

    std::unique_ptr<Core::Component::Data>& dataPtr() {
        return data_;
    }

    std::string name() override {
        return "SkyWalkingTraceSegmentServerEventData";
    }

    void* data() override {
        return data_->data();
    }

    // Fill event with meta,header,body cannot be nil
    void fill(const std::map<std::string, std::string>& /*meta*/, const std::map<std::string, std::string>& /*header*/,
              std::unique_ptr<Core::Component::Data> data) override {
        data_ = std::move(data);
    }

    void addMeta(const std::string& key, const std::string& value) {
        meta_[key] = value;
    }
    void release() override {
    }

    std::string chainName() {
        return "JvmMetricServerEventData";
    }

    ~JvmMetricServerEventData() {
    }

private:
    std::map<std::string, std::string> meta_;
    std::unique_ptr<Core::Component::Data> data_;
};
} // namespace Grpc
} // namespace SkyWalking
} // namespace Source
} // namespace App
