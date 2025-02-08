//
// Created by zhanglei on 25-2-5.
//

#pragma once

#include <memory>

#include <node/v1/info.pb.h>
#include "handler.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include "app/include/source/host/collector/api/collector.h"

// https://github.com/sysstat/sysstat/blob/master/iostat.c
namespace App::Source::Host::Collector::IOStat {
class IOStatCollector : public Api::Collector {
public:
    IOStatCollector() : handler(std::make_unique<Handler>()) {};
    // 运行采集任务
    void collect(novaagent::node::v1::NodeInfo* info);
    // 安装上报
    void run(novaagent::node::v1::NodeInfo* info);

    void start();

    void stop();

private:
    std::unique_ptr<Handler> handler;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter_;
    std::vector<std::unique_ptr<Oltp::MetricCollector>> iostat_metrics_;
};
}