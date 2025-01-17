//
// Created by root on 2024/4/4.
//
#pragma once

#include "app/include/common/opentelemetry/recordable.h"

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Trace {
namespace Skywalking {
class SkyRecordable :public Common::Opentelemetry::Recordable {

public:
    SkyRecordable() = default;
    void SetResource(opentelemetry::sdk::resource::Resource* resource_ptr_) {
        Recordable::SetResource(*resource_ptr_);
        resource_ptr = resource_ptr_;
    }

    void flagFree() {
        isFreeResource = true;
    }

    ~SkyRecordable() override {
        if (isFreeResource && resource_ptr) {
            delete resource_ptr;
        }
    }

private:
    bool isFreeResource = false;
    opentelemetry::sdk::resource::Resource* resource_ptr = nullptr;

};
}
}
}
}
}
