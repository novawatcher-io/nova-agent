#include "app/include/intercept/opentelemetry/metric/skywalking/processor.h"
#include "app/include/common/opentelemetry/const.h"
#include "app/include/common/opentelemetry/otlp_metric_utils.h"
#include "app/include/intercept/opentelemetry/metric/skywalking/sky_metric_resource.h"
#include "common/Common.pb.h"
#include "common/opentelemetry/otlp_event_data.h"
#include "intercept/opentelemetry/metric/skywalking/sky_metric_data.h"
#include "language-agent/JVMMetric.pb.h"
#include <bits/chrono.h>
#include <cstdint>
#include <iterator>
#include <opentelemetry/common/timestamp.h>
#include <opentelemetry/nostd/unique_ptr.h>
#include <opentelemetry/nostd/variant.h>
#include <opentelemetry/sdk/common/attribute_utils.h>
#include <opentelemetry/sdk/metrics/aggregation/aggregation_config.h>
#include <opentelemetry/sdk/metrics/aggregation/histogram_aggregation.h>
#include <opentelemetry/sdk/metrics/data/metric_data.h>
#include <opentelemetry/sdk/metrics/data/point_data.h>
#include <opentelemetry/sdk/metrics/export/metric_producer.h>
#include <opentelemetry/sdk/metrics/instruments.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

using namespace App::Intercept::Opentelemetry::Metric::Skywalking;
using namespace App::Common::Opentelemetry;

opentelemetry::sdk::metrics::HistogramAggregationConfig loadGcHistogramAggregationConfig() {
    opentelemetry::sdk::metrics::HistogramAggregationConfig gcConfig;
    gcConfig.boundaries_ = {0.01, 0.1, 1, 10};
    gcConfig.record_min_max_ = true;
    return gcConfig;
}

const static opentelemetry::sdk::metrics::HistogramAggregationConfig gcHistogramAggregationConfig =
    loadGcHistogramAggregationConfig();

Core::Component::Result Processor::intercept(std::unique_ptr<Core::Component::Batch>& batch) {
    intercept(*batch);
    return {Core::Component::SUCCESS};
}

Core::Component::Result Processor::intercept(Core::Component::Batch& batch) {
    auto& events = batch.events();
    if (events.empty()) {
        return {Core::Component::SUCCESS};
    }
    std::vector<std::unique_ptr<Core::Component::EventData>> agg_list;
    std::vector<std::unique_ptr<OtlpEventData>> recordable_array(batch.events().size());
    for (int i = 0; i < events.size(); i++) {
        auto data = process(events[i]);
        agg_list.push_back(std::move(data));
        //        agg_list.insert(agg_list.end(), std::make_move_iterator(list.begin()),
        //        std::make_move_iterator(list.end()));
    }
    batch.fill(batch.meta(), agg_list);
    return {Core::Component::SUCCESS};
}

const char* getMemoryPoolTyp(skywalking::v3::PoolType type) {
    switch (type) {
    case skywalking::v3::PoolType::CODE_CACHE_USAGE:
        return "CODE_CACHE_USAGE";
    case skywalking::v3::PoolType::NEWGEN_USAGE:
        return "NEWGEN_USAGE";
    case skywalking::v3::PoolType::OLDGEN_USAGE:
        return "OLDGEN_USAGE";
    case skywalking::v3::PoolType::SURVIVOR_USAGE:
        return "SURVIVOR_USAGE";
    case skywalking::v3::PoolType::PERMGEN_USAGE:
        return "PERMGEN_USAGE";
    case skywalking::v3::PoolType::METASPACE_USAGE:
        return "METASPACE_USAGE";
    case skywalking::v3::PoolType::ZHEAP_USAGE:
        return "ZHEAP_USAGE";

    case skywalking::v3::PoolType::COMPRESSED_CLASS_SPACE_USAGE:
        return "COMPRESSED_CLASS_SPACE_USAGE";
    case skywalking::v3::PoolType::CODEHEAP_NON_NMETHODS_USAGE:
        return "CODEHEAP_NON_NMETHODS_USAGE";
    case skywalking::v3::PoolType::CODEHEAP_PROFILED_NMETHODS_USAGE:
        return "CODEHEAP_PROFILED_NMETHODS_USAGE";
    case skywalking::v3::PoolType::CODEHEAP_NON_PROFILED_NMETHODS_USAGE:
        return "CODEHEAP_NON_PROFILED_NMETHODS_USAGE";
    case skywalking::v3::PoolType::PoolType_INT_MIN_SENTINEL_DO_NOT_USE_:
        return "PoolType_INT_MIN_SENTINEL_DO_NOT_USE_";
    case skywalking::v3::PoolType::PoolType_INT_MAX_SENTINEL_DO_NOT_USE_:
        return "PoolType_INT_MAX_SENTINEL_DO_NOT_USE_";
    default:
        return "UNKNOWN";
    }
}

const char* getGCPhase(skywalking::v3::GCPhase phase) {
    switch (phase) {
    case skywalking::v3::GCPhase::NEW:
        return "Young";
    case skywalking::v3::GCPhase::OLD:
        return "Old";
    case skywalking::v3::GCPhase::NORMAL:
        return "NORMAL";
    case skywalking::v3::GCPhase_INT_MIN_SENTINEL_DO_NOT_USE_:
        return "GCPhase_INT_MIN_SENTINEL_DO_NOT_USE_";
    case skywalking::v3::GCPhase_INT_MAX_SENTINEL_DO_NOT_USE_:
        return "GCPhase_INT_MAX_SENTINEL_DO_NOT_USE_";
    }
    return "UNKNOWN";
}

std::unique_ptr<OtlpEventData> Processor::process(std::unique_ptr<Core::Component::EventData>& event) {
    auto object = (skywalking::v3::JVMMetricCollection*)event->data();
    if (object == nullptr) {
        SPDLOG_ERROR("Event data is nullptr");
        return {};
    }
    SPDLOG_DEBUG("Processing JVMMetricCollection for service: {}", object->service());

    opentelemetry::sdk::common::AttributeMap resourceAttr;
    for (auto iter = defaultResource.begin(); iter != defaultResource.end(); iter++) {
        resourceAttr.SetAttribute(AttributeServiceName, object->service());
    }
    resourceAttr.SetAttribute(AttributeServiceName, object->service());
    resourceAttr.SetAttribute(AttributeServiceInstanceID, object->serviceinstance());
    std::unique_ptr<opentelemetry::sdk::resource::Resource> resource =
        std::make_unique<SkyMetricResource>(resourceAttr, "");

    std::vector<metric_sdk::MetricData> list{};
    metric_sdk::HistogramPointData histogram_point_data{};
    histogram_point_data.boundaries_ = std::vector<double>{10.1, 20.2, 30.2};
    histogram_point_data.count_ = 3;
    histogram_point_data.counts_ = {200, 300, 400, 500};
    histogram_point_data.sum_ = 900.5;
    histogram_point_data.min_ = 1.8;
    histogram_point_data.max_ = 12.0;
    metric_sdk::HistogramPointData histogram_point_data2{};
    histogram_point_data2.boundaries_ = std::vector<double>{10.0, 20.0, 30.0};
    histogram_point_data2.count_ = 3;
    histogram_point_data2.counts_ = {200, 300, 400, 500};
    histogram_point_data2.sum_ = static_cast<int64_t>(900);

    for (auto& metric : object->metrics()) {
        auto memoryUsedMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVM::memoryUsed,
                                             Common::Opentelemetry::Metric::JVM::memoryUsedDescription, "By",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        auto memoryCommittedMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVM::memoryCommitted,
                                             Common::Opentelemetry::Metric::JVM::memoryCommittedDescription, "By",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        auto memoryInitedMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVM::memoryInit,
                                             Common::Opentelemetry::Metric::JVM::memoryInitDescription, "By",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        auto memoryLimitMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVM::memoryLimit,
                                             Common::Opentelemetry::Metric::JVM::memoryLimitDescription, "By",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        auto memoryPoolUseMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVM::memoryPoolUse,
                                             Common::Opentelemetry::Metric::JVM::memoryLimitDescription, "By",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        auto memoryPoolLimitMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVM::memoryPoolLimit,
                                             Common::Opentelemetry::Metric::JVM::memoryLimitDescription, "By",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        auto memoryPoolInitMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVM::memoryPoolInit,
                                             Common::Opentelemetry::Metric::JVM::memoryLimitDescription, "By",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        auto memoryPoolCommittedMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVM::memoryPoolCommitted,
                                             Common::Opentelemetry::Metric::JVM::memoryLimitDescription, "By",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        auto& memory = metric.memory();
        for (auto& mem : memory) {
            metric_sdk::LastValuePointData memUsed{};
            memUsed.value_ = mem.used();
            memoryUsedMetric.point_data_attr_.push_back(
                {metric_sdk::PointAttributes{{Common::Opentelemetry::Metric::JVM::memoryType,
                                              mem.isheap() ? App::Common::Opentelemetry::Metric::JVM::heap
                                                           : App::Common::Opentelemetry::Metric::JVM::non_heap}},
                 memUsed});

            metric_sdk::LastValuePointData memCommitted{};
            memCommitted.value_ = mem.committed();
            memoryCommittedMetric.point_data_attr_.push_back(
                {metric_sdk::PointAttributes{{Common::Opentelemetry::Metric::JVM::memoryType,
                                              mem.isheap() ? App::Common::Opentelemetry::Metric::JVM::heap
                                                           : App::Common::Opentelemetry::Metric::JVM::non_heap}},
                 memCommitted});

            metric_sdk::LastValuePointData memInited{};
            memInited.value_ = mem.init();
            memoryInitedMetric.point_data_attr_.push_back(
                {metric_sdk::PointAttributes{{Common::Opentelemetry::Metric::JVM::memoryType,
                                              mem.isheap() ? App::Common::Opentelemetry::Metric::JVM::heap
                                                           : App::Common::Opentelemetry::Metric::JVM::non_heap}},
                 memInited});

            metric_sdk::LastValuePointData memLimit{};
            memLimit.value_ = mem.max();
            memoryLimitMetric.point_data_attr_.push_back(
                {metric_sdk::PointAttributes{{Common::Opentelemetry::Metric::JVM::memoryType,
                                              mem.isheap() ? App::Common::Opentelemetry::Metric::JVM::heap
                                                           : App::Common::Opentelemetry::Metric::JVM::non_heap}},
                 memLimit});
        }

        auto& memoryPoolList = metric.memorypool();
        for (auto& memPool : memoryPoolList) {
            metric_sdk::LastValuePointData memPoolUsed{};
            memPoolUsed.value_ = memPool.used();
            memoryUsedMetric.point_data_attr_.push_back(
                {metric_sdk::PointAttributes{
                     {Common::Opentelemetry::Metric::JVM::memoryPoolName, getMemoryPoolTyp(memPool.type())}},
                 memPoolUsed});

            metric_sdk::LastValuePointData memPoolCommitted{};
            memPoolCommitted.value_ = memPool.committed();
            memoryCommittedMetric.point_data_attr_.push_back(
                {metric_sdk::PointAttributes{
                     {Common::Opentelemetry::Metric::JVM::memoryPoolName, getMemoryPoolTyp(memPool.type())}},
                 memPoolCommitted});

            metric_sdk::LastValuePointData memPoolInited{};
            memPoolInited.value_ = memPool.init();
            memoryInitedMetric.point_data_attr_.push_back(
                {metric_sdk::PointAttributes{
                     {Common::Opentelemetry::Metric::JVM::memoryPoolName, getMemoryPoolTyp(memPool.type())}},
                 memPoolInited});

            metric_sdk::LastValuePointData memPoolLimit{};
            memPoolLimit.value_ = memPool.max();
            memoryLimitMetric.point_data_attr_.push_back(
                {metric_sdk::PointAttributes{
                     {Common::Opentelemetry::Metric::JVM::memoryType, getMemoryPoolTyp(memPool.type())}},
                 memPoolLimit});
        }

        auto& gcList = metric.gc();
        auto gcMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{
                Common::Opentelemetry::Metric::GC::gcDuration, Common::Opentelemetry::Metric::GC::gcDurationDescription,
                "s", metric_sdk::InstrumentType::kHistogram, metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))}};

        for (auto& gc : gcList) {
            opentelemetry::sdk::metrics::DoubleHistogramAggregation aggr(&gcHistogramAggregationConfig);
            auto gcAttr =
                metric_sdk::PointAttributes{{Common::Opentelemetry::Metric::GC::gcName, getGCPhase(gc.phase())}};
            aggr.Aggregate(gc.time(), gcAttr);
            sdk::metrics::HistogramPointData point =
                nostd::get<opentelemetry::sdk::metrics::HistogramPointData>(aggr.ToPoint());
            //            histogramPointData.
            gcMetric.point_data_attr_.push_back({gcAttr, point});
        }

        auto cpuMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::CPU::recent_utilization,
                                             Common::Opentelemetry::Metric::CPU::recentUtilizationDescription, "1",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            std::vector<metric_sdk::PointDataAttributes>{
                {metric_sdk::PointAttributes{}, metric_sdk::LastValuePointData{
                                                    metric.cpu().usagepercent(),
                                                }}}};

        auto threadMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVMThread::count,
                                             Common::Opentelemetry::Metric::JVMThread::countDescription, "1",
                                             metric_sdk::InstrumentType::kUpDownCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            std::vector<metric_sdk::PointDataAttributes>{{metric_sdk::PointAttributes{{
                                                              Common::Opentelemetry::Metric::JVMThread::state,
                                                              "new",
                                                          }},
                                                          metric_sdk::LastValuePointData{
                                                              metric.thread().livecount(),
                                                          }},
                                                         {metric_sdk::PointAttributes{{
                                                              Common::Opentelemetry::Metric::JVMThread::state,
                                                              "runnable",
                                                          }},
                                                          metric_sdk::LastValuePointData{
                                                              metric.thread().runnablestatethreadcount(),
                                                          }},
                                                         {metric_sdk::PointAttributes{{
                                                              Common::Opentelemetry::Metric::JVMThread::state,
                                                              "blocked",
                                                          }},
                                                          metric_sdk::LastValuePointData{
                                                              metric.thread().blockedstatethreadcount(),
                                                          }},
                                                         {metric_sdk::PointAttributes{{
                                                              Common::Opentelemetry::Metric::JVMThread::state,
                                                              "waiting",
                                                          }},
                                                          metric_sdk::LastValuePointData{
                                                              metric.thread().waitingstatethreadcount(),
                                                          }},
                                                         {metric_sdk::PointAttributes{{
                                                              Common::Opentelemetry::Metric::JVMThread::state,
                                                              "timed_waiting",
                                                          }},
                                                          metric_sdk::LastValuePointData{
                                                              metric.thread().timedwaitingstatethreadcount(),
                                                          }},
                                                         {metric_sdk::PointAttributes{{
                                                              Common::Opentelemetry::Metric::JVMThread::state,
                                                              "daemon",
                                                          }},
                                                          metric_sdk::LastValuePointData{
                                                              metric.thread().daemoncount(),
                                                          }},
                                                         {metric_sdk::PointAttributes{{
                                                              Common::Opentelemetry::Metric::JVMThread::state,
                                                              "peak",
                                                          }},
                                                          metric_sdk::LastValuePointData{
                                                              metric.thread().peakcount(),
                                                          }}}};

        auto classLoadMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVMClasses::loaded,
                                             Common::Opentelemetry::Metric::JVMClasses::loadedDescription, "s",
                                             metric_sdk::InstrumentType::kCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            std::vector<metric_sdk::PointDataAttributes>{
                {metric_sdk::PointAttributes{}, metric_sdk::LastValuePointData{
                                                    metric.clazz().loadedclasscount(),
                                                }}}};

        auto classUnLoadMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVMClasses::unloaded,
                                             Common::Opentelemetry::Metric::JVMClasses::unloadeddDescription, "s",
                                             metric_sdk::InstrumentType::kCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            std::vector<metric_sdk::PointDataAttributes>{
                {metric_sdk::PointAttributes{}, metric_sdk::LastValuePointData{
                                                    metric.clazz().totalunloadedclasscount(),
                                                }}}};

        auto classCountMetric = metric_sdk::MetricData{
            metric_sdk::InstrumentDescriptor{Common::Opentelemetry::Metric::JVMClasses::count,
                                             Common::Opentelemetry::Metric::JVMClasses::countDescription, "s",
                                             metric_sdk::InstrumentType::kCounter,
                                             metric_sdk::InstrumentValueType::kDouble},
            metric_sdk::AggregationTemporality::kDelta,
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            opentelemetry::common::SystemTimestamp{
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(metric.time()))},
            std::vector<metric_sdk::PointDataAttributes>{
                {metric_sdk::PointAttributes{}, metric_sdk::LastValuePointData{
                                                    metric.clazz().totalloadedclasscount(),
                                                }}}};

        std::vector<metric_sdk::MetricData> metricVector = {memoryUsedMetric, memoryCommittedMetric,
                                                            memoryInitedMetric};
        list.insert(list.end(), std::move_iterator(metricVector.begin()), std::move_iterator(metricVector.end()));
    }
    std::unique_ptr<SkyMetricData> resourceMetrics = std::make_unique<SkyMetricData>(std::move(resource));
    resourceMetrics->scope_metric_data_ = std::vector<metric_sdk::ScopeMetrics>{{instrumentationScope.get(), list}};
    return std::make_unique<OtlpEventData>(std::move(resourceMetrics));
}
