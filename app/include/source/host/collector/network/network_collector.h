//
// Created by zhanglei on 25-2-1.
//

#pragma once

#include "app/include/source/host/collector/api/collector.h"

// https://github.com/giftnuss/net-tools/blob/master/ifconfig.c
namespace App::Source::Host::Collector::Network {
class NetworkCollector : public Api::Collector {
public:
    void run(novaagent::node::v1::NodeInfo* info) final;
};
}