//
// Created by zhanglei on 2025/2/9.
//

#pragma once

#include <map>
#include <mutex>
#include <string>
#include <memory>
#include <absl/base/thread_annotations.h>

#include "app/include/source/host/collector/oltp/oltp_metric.h"

using App::Source::Host::Collector::Oltp::MultiValue;

namespace App::Source::Host::Collector::Network {
// 缓存设备数据
struct network_device_static_cache {
    unsigned long long rx_packets;	/* total packets received       */
    unsigned long long tx_packets;	/* total packets transmitted    */
    unsigned long long rx_bytes;	/* total bytes received         */
    unsigned long long tx_bytes;	/* total bytes transmitted      */

    double rx_packets_bps;	/* total packets received       */
    double tx_packets_bps;	/* total packets transmitted    */
    double rx_bytes_bps;	/* total bytes received         */
    double tx_bytes_bps;	/* total bytes transmitted      */
};

class CacheStorage {
public:
    // 加载缓存
    void load(struct interface *ife);

    void loadRxPacketsBpsMetric(MultiValue & values);

    void loadRxByteBpsMetric(MultiValue & values);

    void loadTxPacketsBpsMetric(MultiValue & values);

    void loadTxByteBpsMetric(MultiValue & values);

    void loadRxPacketsMetric(MultiValue & values);

    void loadRxByteMetric(MultiValue & values);

    void loadTxPacketsMetric(MultiValue & values);

    void loadTxBytesMetric(MultiValue & values);


    std::mutex mtx;
    // 统计缓存
    std::map<std::string, std::unique_ptr<network_device_static_cache>> caches;
    // 负载时间
    unsigned long long uptime_cs[2] = {0, 0};
    int cur = 1;
};
}