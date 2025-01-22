//
// Created by zhanglei on 25-1-20.
//
#include "app/include/source/http/health_check.h"
#include <http/http_response.h>
namespace App::Source::Http {
void HealthCheck::handle(Core::Http::HttpRequest& request, Core::Http::HttpResponse& response) {
    response.header("Content-Type", "application/json;charset=utf-8");
    response.response(200, "{\"status\":\"UP\"}");
}
}