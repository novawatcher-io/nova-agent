//
// Created by root on 2024/4/16.
//
#include "app/include/common/opentelemetry/otlp_metric_utils.h"
#include <opentelemetry/sdk/instrumentationscope/instrumentation_scope.h>
namespace App {
namespace Common {
namespace Opentelemetry {

proto::metrics::v1::AggregationTemporality OtlpMetricUtils::GetProtoAggregationTemporality(
        const opentelemetry::sdk::metrics::AggregationTemporality &aggregation_temporality) noexcept
{
    if (aggregation_temporality == opentelemetry::sdk::metrics::AggregationTemporality::kCumulative)
        return proto::metrics::v1::AggregationTemporality::AGGREGATION_TEMPORALITY_CUMULATIVE;
    else if (aggregation_temporality == opentelemetry::sdk::metrics::AggregationTemporality::kDelta)
        return proto::metrics::v1::AggregationTemporality::AGGREGATION_TEMPORALITY_DELTA;
    else
        return proto::metrics::v1::AggregationTemporality::AGGREGATION_TEMPORALITY_UNSPECIFIED;
}

void OtlpMetricUtils::ConvertSumMetric(const metric_sdk::MetricData &metric_data,
                                       proto::metrics::v1::Sum *const sum) noexcept
{
    sum->set_aggregation_temporality(
            GetProtoAggregationTemporality(metric_data.aggregation_temporality));
    sum->set_is_monotonic(
            (metric_data.instrument_descriptor.type_ == metric_sdk::InstrumentType::kCounter) ||
            (metric_data.instrument_descriptor.type_ == metric_sdk::InstrumentType::kObservableCounter));
    auto start_ts = metric_data.start_ts.time_since_epoch().count();
    auto ts       = metric_data.end_ts.time_since_epoch().count();
    for (auto &point_data_with_attributes : metric_data.point_data_attr_)
    {
        proto::metrics::v1::NumberDataPoint *proto_sum_point_data = sum->add_data_points();
        proto_sum_point_data->set_start_time_unix_nano(start_ts);
        proto_sum_point_data->set_time_unix_nano(ts);
        auto sum_data = nostd::get<sdk::metrics::SumPointData>(point_data_with_attributes.point_data);

        if ((nostd::holds_alternative<int64_t>(sum_data.value_)))
        {
            proto_sum_point_data->set_as_int(nostd::get<int64_t>(sum_data.value_));
        }
        else
        {
            proto_sum_point_data->set_as_double(nostd::get<double>(sum_data.value_));
        }
        // set attributes
        for (auto &kv_attr : point_data_with_attributes.attributes)
        {
            OtlpPopulateAttributeUtils::PopulateAttribute(proto_sum_point_data->add_attributes(),
                                                          kv_attr.first, kv_attr.second);
        }
    }
}

void OtlpMetricUtils::ConvertHistogramMetric(
        const metric_sdk::MetricData &metric_data,
        proto::metrics::v1::Histogram *const histogram) noexcept
{
    histogram->set_aggregation_temporality(
            GetProtoAggregationTemporality(metric_data.aggregation_temporality));
    auto start_ts = metric_data.start_ts.time_since_epoch().count();
    auto ts       = metric_data.end_ts.time_since_epoch().count();
    for (auto &point_data_with_attributes : metric_data.point_data_attr_)
    {
        proto::metrics::v1::HistogramDataPoint *proto_histogram_point_data =
                histogram->add_data_points();
        proto_histogram_point_data->set_start_time_unix_nano(start_ts);
        proto_histogram_point_data->set_time_unix_nano(ts);
        auto histogram_data =
                nostd::get<sdk::metrics::HistogramPointData>(point_data_with_attributes.point_data);
        // sum
        if ((nostd::holds_alternative<int64_t>(histogram_data.sum_)))
        {
            // Use static_cast to avoid C4244 in MSVC
            proto_histogram_point_data->set_sum(
                    static_cast<double>(nostd::get<int64_t>(histogram_data.sum_)));
        }
        else
        {
            proto_histogram_point_data->set_sum(nostd::get<double>(histogram_data.sum_));
        }
        // count
        proto_histogram_point_data->set_count(histogram_data.count_);
        if (histogram_data.record_min_max_)
        {
            if (nostd::holds_alternative<int64_t>(histogram_data.min_))
            {
                // Use static_cast to avoid C4244 in MSVC
                proto_histogram_point_data->set_min(
                        static_cast<double>(nostd::get<int64_t>(histogram_data.min_)));
            }
            else
            {
                proto_histogram_point_data->set_min(nostd::get<double>(histogram_data.min_));
            }
            if (nostd::holds_alternative<int64_t>(histogram_data.max_))
            {
                // Use static_cast to avoid C4244 in MSVC
                proto_histogram_point_data->set_max(
                        static_cast<double>(nostd::get<int64_t>(histogram_data.max_)));
            }
            else
            {
                proto_histogram_point_data->set_max(nostd::get<double>(histogram_data.max_));
            }
        }
        // buckets

        for (auto bound : histogram_data.boundaries_)
        {
            proto_histogram_point_data->add_explicit_bounds(bound);
        }
        // bucket counts
        for (auto bucket_value : histogram_data.counts_)
        {
            proto_histogram_point_data->add_bucket_counts(bucket_value);
        }
        // attributes
        for (auto &kv_attr : point_data_with_attributes.attributes)
        {
            OtlpPopulateAttributeUtils::PopulateAttribute(proto_histogram_point_data->add_attributes(),
                                                          kv_attr.first, kv_attr.second);
        }
    }
}

void OtlpMetricUtils::ConvertGaugeMetric(const opentelemetry::sdk::metrics::MetricData &metric_data,
                                         proto::metrics::v1::Gauge *const gauge) noexcept
{
    auto start_ts = metric_data.start_ts.time_since_epoch().count();
    auto ts       = metric_data.end_ts.time_since_epoch().count();
    for (auto &point_data_with_attributes : metric_data.point_data_attr_)
    {
        proto::metrics::v1::NumberDataPoint *proto_gauge_point_data = gauge->add_data_points();
        proto_gauge_point_data->set_start_time_unix_nano(start_ts);
        proto_gauge_point_data->set_time_unix_nano(ts);
        auto gauge_data =
                nostd::get<sdk::metrics::LastValuePointData>(point_data_with_attributes.point_data);

        if ((nostd::holds_alternative<int64_t>(gauge_data.value_)))
        {
            proto_gauge_point_data->set_as_int(nostd::get<int64_t>(gauge_data.value_));
        }
        else
        {
            proto_gauge_point_data->set_as_double(nostd::get<double>(gauge_data.value_));
        }
        // set attributes
        for (auto &kv_attr : point_data_with_attributes.attributes)
        {
            OtlpPopulateAttributeUtils::PopulateAttribute(proto_gauge_point_data->add_attributes(),
                                                          kv_attr.first, kv_attr.second);
        }
    }
}


metric_sdk::AggregationType OtlpMetricUtils::GetAggregationType(
        const opentelemetry::sdk::metrics::MetricData &metric_data) noexcept
{
    if (metric_data.point_data_attr_.size() == 0)
    {
        return metric_sdk::AggregationType::kDrop;
    }
    auto point_data_with_attributes = metric_data.point_data_attr_[0];
    if (nostd::holds_alternative<sdk::metrics::SumPointData>(point_data_with_attributes.point_data))
    {
        return metric_sdk::AggregationType::kSum;
    }
    else if (nostd::holds_alternative<sdk::metrics::HistogramPointData>(
            point_data_with_attributes.point_data))
    {
        return metric_sdk::AggregationType::kHistogram;
    }
    else if (nostd::holds_alternative<sdk::metrics::LastValuePointData>(
            point_data_with_attributes.point_data))
    {
        return metric_sdk::AggregationType::kLastValue;
    }
    return metric_sdk::AggregationType::kDrop;
}


void OtlpMetricUtils::PopulateInstrumentInfoMetrics(
        const opentelemetry::sdk::metrics::MetricData &metric_data,
        opentelemetry::proto::metrics::v1::Metric *metric) noexcept
{
    metric->set_name(metric_data.instrument_descriptor.name_);
    metric->set_description(metric_data.instrument_descriptor.description_);
    metric->set_unit(metric_data.instrument_descriptor.unit_);
    auto kind = GetAggregationType(metric_data);
    switch (kind)
    {
        case metric_sdk::AggregationType::kSum: {
            ConvertSumMetric(metric_data, metric->mutable_sum());
            break;
        }
        case metric_sdk::AggregationType::kHistogram: {
            ConvertHistogramMetric(metric_data, metric->mutable_histogram());
            break;
        }
        case metric_sdk::AggregationType::kLastValue: {
            ConvertGaugeMetric(metric_data, metric->mutable_gauge());
            break;
        }
        default:
            break;
    }
}


void OtlpMetricUtils::PopulateRequest(const opentelemetry::sdk::metrics::ResourceMetrics &data,
                                      opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest *request) noexcept {
    if (request == nullptr || data.resource_ == nullptr)
    {
        return;
    }

    auto resource_metrics = request->add_resource_metrics();
    PopulateResourceMetrics(data, resource_metrics);
}

void OtlpMetricUtils::PopulateResourceMetrics(
        const opentelemetry::sdk::metrics::ResourceMetrics &data,
        opentelemetry::proto::metrics::v1::ResourceMetrics *resource_metrics) noexcept
{
    OtlpPopulateAttributeUtils::PopulateAttribute(resource_metrics->mutable_resource(),
                                                  *(data.resource_));

    for (auto &scope_metrics : data.scope_metric_data_)
    {
        if (scope_metrics.scope_ == nullptr)
        {
            continue;
        }
        auto scope_lib_metrics                         = resource_metrics->add_scope_metrics();
        opentelemetry::proto::common::v1::InstrumentationScope *scope = scope_lib_metrics->mutable_scope();
        scope->set_name(scope_metrics.scope_->GetName());
        scope->set_version(scope_metrics.scope_->GetVersion());
        resource_metrics->set_schema_url(scope_metrics.scope_->GetSchemaURL());

        for (auto &metric_data : scope_metrics.metric_data_)
        {
            PopulateInstrumentInfoMetrics(metric_data, scope_lib_metrics->add_metrics());
        }
    }
}
}
}
}