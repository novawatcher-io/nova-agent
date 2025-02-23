//
// Created by root on 2024/4/15.
//
#include "app/include/sink/pipeline/pipeline.h"

using namespace App::Sink::Pipeline;

void Pipeline::addIntercept(std::unique_ptr<Core::Component::Interceptor>& interceptor) {
    chain->addIntercept(std::move(interceptor));
}

void Pipeline::addConsumer(std::unique_ptr<Core::Component::Consumer>& consumer) {
    consumers.push_back(std::move(consumer));
}

void Pipeline::bindChannel(std::shared_ptr<Core::Component::Queue>& queue) {
    channel = queue;
}

void Pipeline::push(std::unique_ptr<Core::Component::EventData> e) {
    std::lock_guard<std::mutex> lock(mutex_);
    chain->addBuffer(std::move(e));
    if (!chain->isFull()) {
        return;
    }
    flush();
}

void Pipeline::flush() {
    if (consumers.empty()) {
        return;
    }
    if (!chain->bufferSize()) {
        return;
    }

    auto& batch_ = chain->flushBatch();
    chain->process(*batch_);
    for (auto i = 0; i < consumers.size(); i++) {
        consumers[i]->Consume(*batch_);
    }
}

void Pipeline::stop() {
    if (consumers.empty()) {
        return;
    }
    for (auto i = 0; i < consumers.size(); i++) {
        consumers[i]->stop();
    }
}