//
// Created by zhanglei on 2025/2/16.
//
#pragma once
#include <list>

#include "listener.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {

class ListenerManager {
public:
    ListenerManager() = default;

    void add (std::unique_ptr<Listener> listener) {
        listeners.push_back(std::move(listener));
    }

    std::list<std::unique_ptr<Listener>>& list() {
        return listeners;
    }

    ~ListenerManager() = default;
private:
    std::list<std::unique_ptr<Listener>> listeners;
};
}
