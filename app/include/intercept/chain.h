//
// Created by root on 2024/3/31.
//

#pragma once

#include <list>
#include <memory>
#include <component/api.h>

#include "app/include/intercept/opentelemetry/trace/skywalking/processor.h"
#include "app/include/sink/channel/batch.h"

namespace App {
namespace Intercept {
class Chain {
public:
    Chain() : b(std::make_unique<App::Sink::Channel::Batch>()) {
    }

    void addIntercept(std::unique_ptr<Core::Component::Interceptor> interceptor) {
        interceptors.push_back(std::move(interceptor));
    }

    void process(std::unique_ptr<Core::Component::Batch>& batch);

    void process(Core::Component::Batch& batch);

    void addBuffer(std::unique_ptr<Core::Component::EventData> data) {
        buffer_.push_back(std::move(data));
    }

    size_t bufferSize() {
        return buffer_.size();
    }

    void clearBatch() {
        b->Release();
    }

    bool isFull() {
        if (buffer_.size() >= maxBuffer) {
            return true;
        }
        return false;
    }

    const std::unique_ptr<Core::Component::Batch>& flushBatch() {
        std::vector<std::unique_ptr<Core::Component::EventData>> buffer;
        buffer_.swap(buffer);
        b->fill(b->meta(), buffer);
        return b;
    }

private:
    std::list<std::unique_ptr<Core::Component::Interceptor>> interceptors;
    std::vector<std::unique_ptr<Core::Component::EventData>> buffer_;
    std::unique_ptr<Core::Component::Batch> b;
    size_t maxBuffer = 0;
};
} // namespace Intercept
} // namespace App
