//
// Created by zhanglei on 2025/2/18.
//

#pragma once

#include "app/include/common/const.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
typedef enum {
    SOURCE = 1,
    DEST = 2
} SERVICE_TYPE;
/**
 * Identify the layer of span's service/instance owner. Such as  ${@link Layer#FAAS} and ${@link Layer#GENERAL}.
 */
static App::Common::Trace::LAYER identifyServiceLayer(int spanLayer) {
    if (spanLayer = App::Common::Trace::LAYER::FAAS) {
        // function as a Service
        return App::Common::Trace::LAYER::FAAS;
    } else {
        return App::Common::Trace::LAYER::GENERAL;
    }
}
}
