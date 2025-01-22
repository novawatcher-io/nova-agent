//
// Created by zhanglei on 25-1-20.
//

#pragma once

#include <component/api.h>
#include <http/http_manager.h>
#include <memory>

#include "config/trace_agent_config.h"
#include "app/include/prometheus/prometheus_exposer.h"


namespace App::Source::Http {

class Source : public Core::Component::Component {
public:
    Source(const std::shared_ptr<Core::Event::EventLoop>& loop_,
       const std::shared_ptr<Core::Component::UnixThreadContainer>& threadPool_,
       const std::shared_ptr<App::Config::ConfigReader>& config,
       std::unique_ptr<App::Prometheus::PrometheusExposer>& exposer);

    std::string name() {
        return Common::Http::http;
    }


    void init() final;

    void start() final;

    void stop() final;

    void finish() final;

    ~Source() override;
private:
    std::unique_ptr<Core::Http::HttpManager> http_manager_;

    std::unique_ptr<App::Prometheus::PrometheusExposer>& exposer_;
};
}