//
// Created by zhanglei on 2025/2/16.
//
#pragma once
#include <list>

#include "listener.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {

class ListenerManager {
public:
    void add (std::unique_ptr<Listener> listener) {
        listeners.emplace_back(std::move(listener));
    }

    std::list<std::unique_ptr<Listener>>& list() {
        return listeners;
    }
private:
    std::list<std::unique_ptr<Listener>> listeners;
};
}
