//
// Created by zhanglei on 25-2-5.
//
#include "app/include/source/host/collector/iostat/iostat_collector.h"
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/provider.h>

using App::Source::Host::Collector::Oltp::MetricCollector;
using App::Source::Host::Collector::Oltp::MultiValue;
using App::Source::Host::Collector::Oltp::SingleValue;

namespace App::Source::Host::Collector::IOStat {

// https://github.com/sysstat/sysstat/blob/master/iostat.c
void IOStatCollector::collect(novaagent::node::v1::NodeInfo* info) {
    handler->stat();
    return;
}

void IOStatCollector::run(novaagent::node::v1::NodeInfo* info) {
    auto provider = opentelemetry::metrics::Provider::GetMeterProvider();
    meter_ = provider->GetMeter("disk_io", "0.0.1");
    iostat_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "disk_io_tps", [&](MultiValue & values) {
        handler->loadAllTpsMetric(values);
    }));

    iostat_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "disk_io_read_bps", [&](MultiValue& values) {
        handler->loadAllReadBpsMetric(values);
    }));

    iostat_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "disk_io_write_bps", [&](MultiValue& values) {
        handler->loadAllWriteBpsMetric(values);
    }));

    iostat_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "disk_io_dscd_bps", [&](MultiValue& values) {
        handler->loadAllDscdBpsMetric(values);
    }));

    iostat_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "disk_io_read_bytes", [&](MultiValue& values) {
        handler->loadAllReadBytesMetric(values);
    }));

    iostat_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "disk_io_write_bytes", [&](MultiValue& values) {
        handler->loadAllWriteBytesMetric(values);
    }));

    iostat_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "disk_io_dscd_bytes", [&](MultiValue& values) {
        handler->loadAllDscdBytesMetric(values);
    }));
}

void IOStatCollector::start() {
    for (auto& metric : iostat_metrics_) {
        metric->Start();
    }
}

void IOStatCollector::stop() {
    for (auto& metric : iostat_metrics_) {
        metric->Start();
    }
}
}