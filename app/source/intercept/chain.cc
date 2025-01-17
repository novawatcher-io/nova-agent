//
// Created by root on 2024/4/1.
//

#include "app/include/intercept/chain.h"

using namespace App::Intercept;

void Chain::process(std::unique_ptr<Core::Component::Batch>& batch) {
    for (auto& i : interceptors) {
        auto ret = i->intercept(batch);
        if (ret.status() == Core::Component::FAIL) {
            std::cerr << ret.message_ << std::endl;
            break;
        }
    }
}

void Chain::process(Core::Component::Batch& batch) {
    for (auto& i : interceptors) {
        auto ret = i->intercept(batch);
        if (ret.status() == Core::Component::FAIL) {
            std::cerr << ret.message_ << std::endl;
            break;
        }
    }
}
