//
// Created by zhanglei on 2025/3/1.
//

#pragma once

#include <nova_agent_payload/trace/v1/topology.pb.h>

#include <mutex>
#include <map>

#include "app/include/common/lrucache.h"
#include "rpc_traffic_source_builder.h"
#include "app/include/sink/topology/sink.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
class ServiceMetricAggregator {
public:
    ServiceMetricAggregator(uint32_t maxSize_, const std::unique_ptr<Sink::Topology::Sink> &sink);

    void stats(const std::unique_ptr<RPCTrafficSourceBuilder>& builder);

    void send();

    ~ServiceMetricAggregator();

private:
    void loadMetric(uint64_t id, const std::unique_ptr<RPCTrafficSourceBuilder>& builder, int flags);

    std::map<uint64_t, std::unique_ptr<novaagent::trace::v1::ServiceMetric>> metricCache;
    std::mutex mtx;
    uint32_t maxSize = 0;
    uint32_t batchSize = 100;
    const std::unique_ptr<Sink::Topology::Sink> &sink_;
};
}
