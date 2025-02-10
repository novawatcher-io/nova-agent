//
// Created by zhanglei on 25-2-1.
//

#pragma once

#include <map>
#include <mutex>

#include "app/include/source/host/collector/api/collector.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include "socket.h"
#include "network_interface.h"
#include "cache_storage.h"

// https://github.com/giftnuss/net-tools/blob/master/ifconfig.c
namespace App::Source::Host::Collector::Network {
class NetworkCollector : public Api::Collector {
public:
    NetworkCollector()
    :socket_(std::make_unique<Socket>()), interface_(std::make_unique<NetworkInterface>(socket_)),
    storage(std::make_unique<CacheStorage>()) {};

    void run(novaagent::node::v1::NodeInfo* info) final;

    void install(novaagent::node::v1::NodeInfo* info) final;

    void start() final;

    void stop() final;

private:
    int doit(struct interface *ife, void *cookie, void* ptr);
    std::unique_ptr<NetworkInterface> interface_;
    std::unique_ptr<Socket> socket_;
    // 计数器用来统计每秒的网络流量
    int cur = 1;

    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter_;
    std::vector<std::unique_ptr<Oltp::MetricCollector>> network_device_metrics_;

    std::unique_ptr<CacheStorage> storage;
};
}