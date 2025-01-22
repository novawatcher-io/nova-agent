//
// Created by zhanglei on 25-1-20.
//
#pragma once
#include <core.h>
#include <http/http_action.h>
#include "common/const.h"
namespace App::Source::Http {
class HealthCheck :public Core::Noncopyable, public Core::Nonmoveable {
public:
    HealthCheck() = default;
    void handle(Core::Http::HttpRequest &request, Core::Http::HttpResponse &response);
    ~HealthCheck() = default;
    std::string path() {
        return Common::Http::health_path;
    }
};
}