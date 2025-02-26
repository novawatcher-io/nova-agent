//
// Created by zhanglei on 2025/2/24.
//
#pragma once

#include <component/api.h>
#include "config/nova_agent_config.h"
#include "watcher.h"

namespace App::Source::Kubernetes {
class Source : public Core::Component::Component {
public:
    Source(std::shared_ptr<App::Config::ConfigReader>& config)
    :watcher(std::make_unique<Watcher>(config)), config_(config) {

    }
    void init() final;

    void start() final;

    void stop() final;

    void finish() final;

    std::string name() final;

    ~Source() override;
private:
    std::unique_ptr<Watcher> watcher;
    const std::shared_ptr<App::Config::ConfigReader>& config_;
};
}