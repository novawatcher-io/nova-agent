//
// Created by zhanglei on 2025/3/1.
//
#include "app/include/intercept/opentelemetry/trace/topology/service_relation_metric_aggregator.h"

#include <spdlog/spdlog.h>

#include "app/include/common/machine.h"
#include "app/include/intercept/opentelemetry/trace/topology/common.h"

namespace App::Intercept::Opentelemetry::Trace::Topology {

ServiceRelationMetricAggregator::ServiceRelationMetricAggregator(uint32_t maxSize_, const std::unique_ptr<Sink::Topology::Sink> &sink)
:maxSize(maxSize_), sink_(sink) {

}

void ServiceRelationMetricAggregator::stats(const std::unique_ptr<RPCTrafficSourceBuilder>& builder) {
    std::lock_guard guard(mtx);
    if (metricCache.size() > maxSize) {
        SPDLOG_DEBUG("metricCache max size({}) is full", maxSize);
        return;
    }
    if (builder->relation_id > 0) {
        loadMetric(builder->relation_id, builder);
    }
}

void ServiceRelationMetricAggregator::loadMetric(uint64_t id, const std::unique_ptr<RPCTrafficSourceBuilder>& builder) {
    auto iter = metricCache.find(id);

    if (iter == metricCache.end()) {
        metricCache[id] = std::make_unique<novaagent::trace::v1::ServiceRelationMetric>();
        iter = metricCache.find(id);
    }

    auto& metric = iter->second;
    metric->set_id(id);
    metric->set_sourceservicename(builder->sourceServiceName);
    metric->set_sourcenamespace(builder->sourceNamespace);
    metric->set_sourceserviceinstancename(builder->sourceServiceInstanceName);
    metric->set_sourcelayer(builder->sourceLayer);
    metric->set_destservicename(builder->destServiceName);
    metric->set_destnamespace(builder->destNamespace);
    metric->set_destserviceinstancename(builder->destServiceInstanceName);
    metric->set_destlayer(builder->destLayer);
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
    metricCache[id] = std::move(metric);
}

void ServiceRelationMetricAggregator::send() {
    std::lock_guard guard(mtx);
    auto iter = metricCache.begin();
    if (metricCache.empty()) {
        return;
    }

    novaagent::trace::v1::ServiceRelationMetricRequest request;
    uint64_t sum = 0;
    for (; iter != metricCache.end(); iter++) {
        auto metric = request.add_servicerelationmetric();
        metric->set_id(iter->second->id());
        metric->set_objectid(Common::getMachineId());
        metric->set_sourceservicename(iter->second->sourceservicename());
        metric->set_sourcenamespace(iter->second->sourcenamespace());
        metric->set_sourceserviceinstancename(iter->second->sourceserviceinstancename());
        metric->set_sourcelayer(iter->second->sourcelayer());
        metric->set_destservicename(iter->second->destservicename());
        metric->set_destnamespace(iter->second->destnamespace());
        metric->set_destserviceinstancename(iter->second->destserviceinstancename());
        metric->set_destlayer(iter->second->destlayer());
        metric->set_cluster(iter->second->cluster());
        metric->set_request_count(iter->second->request_count());
        metric->set_error_count(iter->second->error_count());
        metric->set_avg_time(iter->second->avg_time());
        metric->set_objectid(Common::getMachineId());
        std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::system_clock::now().time_since_epoch()
        );
        metric->set_time(ms.count());

        if (sum > batchSize) {
            sink_->RegisterServiceRelationMetric(request);
            request.clear_servicerelationmetric();
        } else {
            if (sum == metricCache.size() - 1) {
                 sink_->RegisterServiceRelationMetric(request);
            }
        }
        sum++;
    }
    metricCache.clear();
}


ServiceRelationMetricAggregator::~ServiceRelationMetricAggregator() {}

}