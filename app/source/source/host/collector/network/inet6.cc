//
// Created by zhanglei on 25-2-1.
//
#include "app/include/source/host/collector/network/af_type.h"
#include "app/include/source/host/collector/network/inet6.h"
#include "app/include/source/host/collector/network/inet6_sr.h"
#include "app/include/source/host/collector/network/inet6_gr.h"

namespace App::Source::Host::Collector::Network {
struct aftype inet6_aftype =
{
    "inet6", NULL, /*"IPv6", */ AF_INET6, sizeof(struct in6_addr),
    INET6_print, INET6_sprint, INET6_input, INET6_reserror,
    INET6_rprint, INET6_rinput, NULL,

    -1,
    "/proc/net/if_inet6"
};
}