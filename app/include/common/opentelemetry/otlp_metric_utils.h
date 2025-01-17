
#pragma once

#include <opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h>
#include <opentelemetry/proto/metrics/v1/metrics.pb.h>
#include <opentelemetry/proto/resource/v1/resource.pb.h>

#include "otlp_populate_attribute_utils.h"
#include <opentelemetry/sdk/metrics/export/metric_producer.h>

namespace App {
namespace Common {
namespace Opentelemetry {
using namespace opentelemetry;
namespace metric_sdk = opentelemetry::sdk::metrics;
/**
 * The OtlpMetricUtils contains utility functions for OTLP metrics
 */
class OtlpMetricUtils {
public:
    static proto::metrics::v1::AggregationTemporality GetProtoAggregationTemporality(
        const opentelemetry::sdk::metrics::AggregationTemporality& aggregation_temporality) noexcept;
    static metric_sdk::AggregationType
    GetAggregationType(const opentelemetry::sdk::metrics::MetricData& metric_data) noexcept;

    static void
    PopulateRequest(const opentelemetry::sdk::metrics::ResourceMetrics& data,
                    opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest* request) noexcept;

    static void PopulateInstrumentInfoMetrics(const opentelemetry::sdk::metrics::MetricData& metric_data,
                                              opentelemetry::proto::metrics::v1::Metric* metric) noexcept;

    static void ConvertSumMetric(const opentelemetry::sdk::metrics::MetricData& metric_data,
                                 proto::metrics::v1::Sum* const sum) noexcept;

    static void ConvertHistogramMetric(const opentelemetry::sdk::metrics::MetricData& metric_data,
                                       proto::metrics::v1::Histogram* const histogram) noexcept;

    static void ConvertGaugeMetric(const opentelemetry::sdk::metrics::MetricData& metric_data,
                                   proto::metrics::v1::Gauge* const gauge) noexcept;

    static void PopulateResourceMetrics(const opentelemetry::sdk::metrics::ResourceMetrics& data,
                                        opentelemetry::proto::metrics::v1::ResourceMetrics* resource_metrics) noexcept;
};

}
}
}
