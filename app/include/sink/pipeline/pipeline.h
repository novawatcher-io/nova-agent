//
// Created by root on 2024/4/15.
//
#pragma once
#include <vector>
#include "app/include/intercept/chain.h"

namespace App {
namespace Sink {
namespace Pipeline {
class Pipeline :public Core::Component::Pipeline {
public:
    Pipeline() : chain(std::make_unique<Intercept::Chain>()) {
    }
    ~Pipeline() = default;
    void addIntercept(std::unique_ptr<Core::Component::Interceptor>& interceptor) final;

    void addConsumer(std::unique_ptr<Core::Component::Consumer>& consumer) final;

    void bindChannel(std::shared_ptr<Core::Component::Queue>& queue) final;

    void push(std::unique_ptr<Core::Component::EventData> e) final;

    void flush() final;
private:
    std::vector<std::unique_ptr<Core::Component::Consumer>> consumers;
    std::unique_ptr<Intercept::Chain> chain;
    std::shared_ptr<Core::Component::Queue> channel;
    std::mutex mutex_;
};
}
}
}