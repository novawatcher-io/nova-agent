//
// Created by zhanglei on 2025/2/9.
//

#include "app/include/source/host/collector/network/cache_storage.h"
#include "app/include/source/host/collector/network/network_interface.h"
#include "app/include/common/util.h"

namespace App::Source::Host::Collector::Network {

void CacheStorage::load(struct interface *ife) {
     std::lock_guard<std::mutex> lock(mtx);
     /**
     * 计算bps
     */
    // 读取uptime
    Common::read_uptime(&(uptime_cs[cur]));

    /* Calculate time interval in 1/100th of a second */
    unsigned long long itv = Common::get_interval(uptime_cs[!cur], uptime_cs[cur]);

    auto iter = caches.find(ife->name);
    if (iter == caches.end()) {
        auto cache = std::make_unique<network_device_static_cache>();
        cache->rx_bytes = 0;
        cache->tx_bytes = 0;
        cache->rx_packets = 0;
        cache->tx_packets = 0;
        caches[ife->name] = std::move(cache);
        iter = caches.find(ife->name);
    }
    iter->second->rx_packets_bps = (ife->stats.rx_packets - iter->second->rx_packets) / itv;
    iter->second->tx_packets_bps = (ife->stats.tx_packets - iter->second->tx_packets) / itv;

    iter->second->rx_bytes_bps = (ife->stats.rx_bytes - iter->second->rx_bytes) / itv;
    iter->second->tx_bytes_bps = (ife->stats.tx_bytes - iter->second->tx_bytes) / itv;

    iter->second->rx_bytes = ife->stats.rx_bytes;
    iter->second->tx_bytes = ife->stats.rx_bytes;
    iter->second->rx_packets = ife->stats.rx_packets;
    iter->second->tx_packets = ife->stats.tx_packets;

    // 缓存上次的指标
    cur ^= 1;
}

void CacheStorage::loadRxPacketsBpsMetric(MultiValue & values) {
    std::lock_guard<std::mutex> lock(mtx);
    if (caches.empty()) {
        return;
    }
    values.resize(caches.size());

    int count = 0;
    for (const auto& data : caches) {
        values[count].data = double(data.second->rx_packets_bps);
        values[count].labels["dev_name"] = data.first;
        count++;
    }
    return;
}

void CacheStorage::loadRxByteBpsMetric(MultiValue & values) {
    std::lock_guard<std::mutex> lock(mtx);
    if (caches.empty()) {
        return;
    }
    values.resize(caches.size());

    int count = 0;
    for (const auto& data : caches) {
        values[count].data = double(data.second->rx_bytes_bps);
        values[count].labels["dev_name"] = data.first;
        count++;
    }
    return;
}

void CacheStorage::loadTxPacketsBpsMetric(MultiValue & values) {
    std::lock_guard<std::mutex> lock(mtx);
    if (caches.empty()) {
        return;
    }
    values.resize(caches.size());

    int count = 0;
    for (const auto& data : caches) {
        values[count].data = double(data.second->tx_packets_bps);
        values[count].labels["dev_name"] = data.first;
        count++;
    }
    return;
}

void CacheStorage::loadTxByteBpsMetric(MultiValue & values) {
    std::lock_guard<std::mutex> lock(mtx);
    if (caches.empty()) {
        return;
    }
    values.resize(caches.size());

    int count = 0;
    for (const auto& data : caches) {
        values[count].data = double(data.second->tx_bytes_bps);
        values[count].labels["dev_name"] = data.first;
        count++;
    }
    return;
}

void CacheStorage::loadRxPacketsMetric(MultiValue & values) {
    std::lock_guard<std::mutex> lock(mtx);
    if (caches.empty()) {
        return;
    }
    values.resize(caches.size());

    int count = 0;
    for (const auto& data : caches) {
        values[count].data = double(data.second->rx_packets);
        values[count].labels["dev_name"] = data.first;
        count++;
    }
    return;
}

void CacheStorage::loadRxByteMetric(MultiValue & values) {
    std::lock_guard<std::mutex> lock(mtx);
    if (caches.empty()) {
        return;
    }
    values.resize(caches.size());

    int count = 0;
    for (const auto& data : caches) {
        values[count].data = double(data.second->rx_bytes);
        values[count].labels["dev_name"] = data.first;
        count++;
    }
    return;
}

void CacheStorage::loadTxPacketsMetric(MultiValue & values) {
    std::lock_guard<std::mutex> lock(mtx);
    if (caches.empty()) {
        return;
    }
    values.resize(caches.size());

    int count = 0;
    for (const auto& data : caches) {
        values[count].data = double(data.second->tx_packets);
        values[count].labels["dev_name"] = data.first;
        count++;
    }
    return;
}

void CacheStorage::loadTxBytesMetric(MultiValue & values) {
    std::lock_guard<std::mutex> lock(mtx);
    if (caches.empty()) {
        return;
    }
    values.resize(caches.size());

    int count = 0;
    for (const auto& data : caches) {
        values[count].data = double(data.second->tx_bytes);
        values[count].labels["dev_name"] = data.first;
        count++;
    }
    return;
}
}