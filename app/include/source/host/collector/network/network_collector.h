//
// Created by zhanglei on 25-2-1.
//

#pragma once

#include "app/include/source/host/collector/api/collector.h"
#include "socket.h"
#include "network_interface.h"

// https://github.com/giftnuss/net-tools/blob/master/ifconfig.c
namespace App::Source::Host::Collector::Network {
class NetworkCollector : public Api::Collector {
public:
    NetworkCollector()
    :socket_(std::make_unique<Socket>()), interface_(std::make_unique<NetworkInterface>(socket_)) {};
    void run(novaagent::node::v1::NodeInfo* info) final;

private:
    std::unique_ptr<NetworkInterface> interface_;
    std::unique_ptr<Socket> socket_;
};
}