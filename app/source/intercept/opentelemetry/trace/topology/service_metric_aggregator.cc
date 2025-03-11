//
// Created by zhanglei on 2025/3/1.
//
#include "app/include/intercept/opentelemetry/trace/topology/service_metric_aggregator.h"

#include <spdlog/spdlog.h>
#include "app/include/common/machine.h"
#include "app/include/intercept/opentelemetry/trace/topology/common.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {
ServiceMetricAggregator::ServiceMetricAggregator(uint32_t maxSize_, const std::unique_ptr<Sink::Topology::Sink> &sink)
:maxSize(maxSize_), sink_(sink) {

}

void ServiceMetricAggregator::stats(const std::unique_ptr<RPCTrafficSourceBuilder>& builder) {
    std::lock_guard guard(mtx);
    if (metricCache.size() > maxSize) {
        SPDLOG_DEBUG("metricCache max size({}) is full", maxSize);
        return;
    }
    if (builder->getSourceId() > 0) {
//        loadMetric(builder->getSourceId(), builder, SERVICE_TYPE::SOURCE);
    }

    if (builder->getDestId() > 0) {
        loadMetric(builder->getDestId(), builder, SERVICE_TYPE::DEST);
    }
}

void ServiceMetricAggregator::loadMetric(uint64_t id, const std::unique_ptr<RPCTrafficSourceBuilder>& builder, int flags) {
    auto iter = metricCache.find(id);

    if (iter == metricCache.end()) {
        metricCache[id] = std::make_unique<novaagent::trace::v1::ServiceMetric>();
        iter = metricCache.find(id);
    }

    auto& metric = iter->second;
    metric->set_id(id);
    if (flags == SERVICE_TYPE::SOURCE) {
        if (builder->sourceServiceName.empty()) {
            return;
        }
        metric->set_name(builder->sourceServiceName);
        metric->set_namespace_(builder->sourceNamespace);
    } else {
        if (builder->destServiceName.empty()) {
            return;
        }
        metric->set_name(builder->destServiceName);
        metric->set_namespace_(builder->destNamespace);
    }
    metric->set_cluster(builder->cluster);
    uint64_t request_count;
    request_count = metric->request_count();
    request_count++;
    metric->set_request_count(request_count);
    if (builder->hasError) {
      uint64_t error_count = metric->error_count();
      error_count++;
      metric->set_error_count(error_count);
    }
    uint64_t latency = metric->avg_time();
    double avg_time = (double)(latency + builder->latency) / 2;
    metric->set_avg_time(avg_time);
}

ServiceMetricAggregator::~ServiceMetricAggregator() {

}

void ServiceMetricAggregator::send() {
    std::lock_guard guard(mtx);
    auto iter = metricCache.begin();
    if (metricCache.empty()) {
        return;
    }

    novaagent::trace::v1::ServiceMetricRequest request;
    uint64_t sum = 0;
    for (; iter != metricCache.end(); iter++) {
        auto metric = request.add_servicemetric();
        metric->set_id(iter->second->id());
        metric->set_name(iter->second->name());
        metric->set_namespace_(iter->second->namespace_());
        metric->set_cluster(iter->second->cluster());
        metric->set_objectid(Common::getMachineId());
        metric->set_request_count(iter->second->request_count());
        metric->set_error_count(iter->second->error_count());
        metric->set_avg_time(iter->second->avg_time());
        std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::system_clock::now().time_since_epoch()
        );
        metric->set_time(ms.count());

        if (sum > batchSize) {
            sink_->RegisterServiceMetric(request);
            request.clear_servicemetric();
        } else {
            if (sum == metricCache.size() - 1) {
                 sink_->RegisterServiceMetric(request);
            }
        }
        sum++;
    }
    metricCache.clear();
}
}