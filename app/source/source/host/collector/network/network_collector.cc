//
// Created by zhanglei on 25-2-1.
//
#include "app/include/source/host/collector/network/network_collector.h"
#include "app/include/source/host/collector/network/af.h"
#include "app/include/source/host/collector/network/pathname.h"
#include "app/include/source/host/collector/network/ipx.h"
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/provider.h>

using App::Source::Host::Collector::Oltp::MetricCollector;
using App::Source::Host::Collector::Oltp::MultiValue;
using App::Source::Host::Collector::Oltp::SingleValue;

namespace App::Source::Host::Collector::Network {

int NetworkCollector::doit(struct interface *ife, void *cookie, void* ptr)
{
    if (!cookie) {
        return -1;
    }

    if (!ptr) {
        return -1;
    }
    auto interface_ptr = (NetworkInterface *)ptr;
    auto session = (novaagent::node::v1::NodeInfo*) cookie;
    int res;
    struct aftype *ap;
    FILE *f;
    char addr6p[8][5];
    static char flags[200];

    res = interface_ptr->fetch(ife);
    if (res >= 0) {
        if ((ife->flags & IFF_UP)) {
            auto interface_info = session->add_network_interface_list();
            sprintf(flags, "flags=%d<", ife->flags);
            char addr6[40], devname[21];
            struct sockaddr_in6 sap;
            int plen, scope, dad_status, if_idx;
            extern struct aftype inet6_aftype;

            interface_info->set_name(ife->name);
            interface_info->set_mtu(ife->mtu);
            interface_info->set_metric(ife->metric);
            if (ife->statistics_valid) {
                auto stats = interface_info->mutable_stats();
                stats->set_rx_packets(ife->stats.rx_packets);
                stats->set_tx_packets(ife->stats.tx_packets);

                stats->set_rx_bytes(ife->stats.rx_bytes);
                stats->set_tx_bytes(ife->stats.tx_bytes);

                stats->set_rx_errors(ife->stats.rx_errors);
                stats->set_tx_errors(ife->stats.tx_errors);

                stats->set_rx_dropped(ife->stats.rx_dropped);
                stats->set_tx_dropped(ife->stats.tx_dropped);

                stats->set_rx_multicast(ife->stats.rx_multicast);
                stats->set_rx_compressed(ife->stats.rx_compressed);
                stats->set_tx_compressed(ife->stats.tx_compressed);

                stats->set_collisions(ife->stats.collisions);

                stats->set_rx_length_errors(ife->stats.rx_length_errors);

                stats->set_rx_over_errors(ife->stats.rx_over_errors);
                stats->set_rx_crc_errors(ife->stats.rx_crc_errors);

                stats->set_rx_frame_errors(ife->stats.rx_frame_errors);
                stats->set_rx_fifo_errors(ife->stats.rx_fifo_errors);
                stats->set_rx_missed_errors(ife->stats.rx_missed_errors);
                stats->set_tx_aborted_errors(ife->stats.tx_aborted_errors);
                stats->set_tx_carrier_errors(ife->stats.tx_carrier_errors);
                stats->set_tx_fifo_errors(ife->stats.tx_fifo_errors);
                stats->set_tx_heartbeat_errors(ife->stats.tx_heartbeat_errors);
                stats->set_tx_window_errors(ife->stats.tx_window_errors);
            }
            interface_info->set_flags(ife->flags);

            ap = get_afntype(ife->addr.sa_family);
            if (ap == NULL)
                ap = get_afntype(0);
            if (ife->has_ip) {
                interface_info->set_ip(ap->sprint(&ife->addr, 1));
            }

            if ((f = fopen(_PATH_PROCNET_IFINET6, "r")) != NULL) {
                while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n",
                              addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                              addr6p[4], addr6p[5], addr6p[6], addr6p[7],
                          &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
                    if (!strcmp(devname, ife->name.c_str())) {
                        sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
                                addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                                addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
                        inet6_aftype.input(1, addr6, (struct sockaddr *) &sap);

                        flags[0] = '<'; flags[1] = 0;
                        if (scope & IPV6_ADDR_COMPATv4) {
                            strcat(flags, "compat,");
                            scope -= IPV6_ADDR_COMPATv4;
                        }
                        if (scope == 0)
                            strcat(flags, "global,");
                        if (scope & IPV6_ADDR_LINKLOCAL)
                            strcat(flags, "link,");
                        if (scope & IPV6_ADDR_SITELOCAL)
                            strcat(flags, "site,");
                        if (scope & IPV6_ADDR_LOOPBACK)
                            strcat(flags, "host,");
                        if (flags[strlen(flags)-1] == ',')
                            flags[strlen(flags)-1] = '>';
                        else
                            flags[strlen(flags)-1] = 0;
                        interface_info->set_ip6(addr6);
                    }
                }
                fclose(f);
            }

            storage->load(ife);
            return 0;
        }
    }
    return res;
}


void NetworkCollector::run(novaagent::node::v1::NodeInfo* info) {
    interface_->collect(
            std::bind(&NetworkCollector::doit, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                        info, interface_.get()
    );
}

void NetworkCollector::install(novaagent::node::v1::NodeInfo *info) {
    auto provider = opentelemetry::metrics::Provider::GetMeterProvider();
    meter_ = provider->GetMeter("network_io", "0.0.1");
    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "rx_packets", [&](MultiValue & values) {
        storage->loadRxPacketsMetric(values);
    }));

    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "rx_dev_packets", [&](MultiValue & values) {
        storage->loadRxPacketsMetric(values);
    }));

    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "tx_packets", [&](MultiValue& values) {
        storage->loadTxPacketsMetric(values);
    }));

    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "rx_bytes", [&](MultiValue& values) {
        storage->loadRxByteMetric(values);
    }));

    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "tx_bytes", [&](MultiValue& values) {
        storage->loadTxBytesMetric(values);
    }));

    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "rx_packets_bps", [&](MultiValue& values) {
        storage->loadRxPacketsBpsMetric(values);
    }));

    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "tx_packets_bps", [&](MultiValue& values) {
        storage->loadTxPacketsBpsMetric(values);
    }));

    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "rx_bytes_bps", [&](MultiValue& values) {
        storage->loadRxByteBpsMetric(values);
    }));

    network_device_metrics_.emplace_back(MetricCollector::Create<double>(meter_, info->company_uuid(), "tx_bytes_bps", [&](MultiValue& values) {
        storage->loadTxByteBpsMetric(values);
    }));
}

void NetworkCollector::start() {
    for (auto& metric : network_device_metrics_) {
        metric->Start();
    }
}

void NetworkCollector::stop() {
    for (auto& metric : network_device_metrics_) {
        metric->Start();
    }
}
}