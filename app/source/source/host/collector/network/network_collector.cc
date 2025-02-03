//
// Created by zhanglei on 25-2-1.
//
#include "app/include/source/host/collector/network/network_collector.h"

namespace App::Source::Host::Collector::Network {

void NetworkCollector::run(novaagent::node::v1::NodeInfo* info) {
    interface_->all();
}
}