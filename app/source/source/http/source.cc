//
// Created by zhanglei on 25-1-20.
//

#include "app/include/source/http/source.h"
#include "app/include/source/http/health_check.h"

#include <http/http_router.h>

namespace App::Source::Http {
Source::Source(const std::shared_ptr<Core::Event::EventLoop>& loop_,
       const std::shared_ptr<Core::Component::UnixThreadContainer>& threadPool_,
       const std::shared_ptr<App::Config::ConfigReader>& config,
       std::unique_ptr<App::Prometheus::PrometheusExposer>& exposer) :exposer_(exposer) {
    auto http_config = std::make_shared<Core::Http::HttpConfig>();
    if (config->GetConfig().has_http_server_config()) {
        http_config->setIp(config->GetConfig().http_server_config().address());
        http_config->setPort(config->GetConfig().http_server_config().port());
        // http_config->setTimeout(config->GetConfig().http_server_config().timeout());
    }
    http_manager_ = std::make_unique<Core::Http::HttpManager>(loop_, threadPool_, http_config);
}

void Source::init() {
    http_manager_->init();
}

void Source::start() {
    std::shared_ptr<Core::Http::HttpAction> action = std::make_shared<Core::Http::HttpAction>();

    //绑定 health action
    auto healthChecker = std::make_shared<HealthCheck>();
    action->setUsers(std::bind(&HealthCheck::handle, healthChecker, std::placeholders::_1, std::placeholders::_2));
    http_manager_->getRouter()->getRequest(healthChecker->path(), action);

    //绑定 metric action
    std::shared_ptr<Core::Http::HttpAction> metricAction = std::make_shared<Core::Http::HttpAction>();
    metricAction->setUsers([this](auto && PH1, auto && PH2) { exposer_->handle(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    http_manager_->getRouter()->getRequest(Common::Http::metric_path, metricAction);

    http_manager_->start();
}

void Source::stop() {
    http_manager_->stop();
}

void Source::finish() {
    http_manager_->finish();
}

Source::~Source() {

}


}