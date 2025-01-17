//
// Created by root on 2024/4/3.
//

#pragma once

#include <component/api.h>

namespace App {
namespace Common {
namespace Opentelemetry {
class OtlpEventData : public Core::Component::EventData {
public:
    explicit OtlpEventData(std::unique_ptr<Core::Component::Data> data) : data_(std::move(data)) {
    }
    void* data() override {
        return data_.get();
    }
    std::unique_ptr<Core::Component::Data>& dataPtr() override {
        return data_;
    }
    std::map<std::string, std::string> meta() override {
        return meta_;
    }
    std::map<std::string, std::string> header() override {
        return {};
    }
    std::string name() override {
        return "";
    }
    // Fill event with meta,header,body cannot be nil
    void fill(const std::map<std::string, std::string>& , const std::map<std::string, std::string>& ,
              std::unique_ptr<Core::Component::Data> data) override {
        data_ = std::move(data);
        return;
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        meta_ = meta;
    }
    void release() override {
    }

    [[maybe_unused]] ~OtlpEventData() = default;

private:
    std::map<std::string, std::string> meta_;
    std::unique_ptr<Core::Component::Data> data_;
};
}
}
}
