//
// Created by zhanglei on 25-2-3.
//

#pragma once

extern "C" {
#include <string.h>
#include <netinet/in.h>
#include <net/if.h>
}

#include <memory>
#include <functional>
#include <string>
#include <map>
#include "socket.h"
#include "cache_storage.h"

namespace App::Source::Host::Collector::Network {

struct user_net_device_stats {
    unsigned long long rx_packets;	/* total packets received       */
    unsigned long long tx_packets;	/* total packets transmitted    */
    unsigned long long rx_bytes;	/* total bytes received         */
    unsigned long long tx_bytes;	/* total bytes transmitted      */
    unsigned long rx_errors;	/* bad packets received         */
    unsigned long tx_errors;	/* packet transmit problems     */
    unsigned long rx_dropped;	/* no space in linux buffers    */
    unsigned long tx_dropped;	/* no space available in linux  */
    unsigned long rx_multicast;	/* multicast packets received   */
    unsigned long rx_compressed;
    unsigned long tx_compressed;
    unsigned long collisions;

    /* detailed rx_errors: */
    unsigned long rx_length_errors;
    unsigned long rx_over_errors;	/* receiver ring buff overflow  */
    unsigned long rx_crc_errors;	/* recved pkt with crc error    */
    unsigned long rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long rx_missed_errors;	/* receiver missed packet     */
    /* detailed tx_errors */
    unsigned long tx_aborted_errors;
    unsigned long tx_carrier_errors;
    unsigned long tx_fifo_errors;
    unsigned long tx_heartbeat_errors;
    unsigned long tx_window_errors;
};

struct interface {
public:
    std::string name;	/* interface name        */
    short type;			/* if type               */
    short flags;		/* various flags         */
    int metric;			/* routing metric        */
    int mtu;			/* MTU value             */
    int tx_queue_len;		/* transmit queue length */
    struct ifmap map;		/* hardware setup        */
    struct sockaddr addr;	/* IP address            */
    struct sockaddr dstaddr;	/* P-P IP address        */
    struct sockaddr broadaddr;	/* IP broadcast address  */
    struct sockaddr netmask;	/* IP network mask       */
    struct sockaddr ipxaddr_bb;	/* IPX network address   */
    struct sockaddr ipxaddr_sn;	/* IPX network address   */
    struct sockaddr ipxaddr_e3;	/* IPX network address   */
    struct sockaddr ipxaddr_e2;	/* IPX network address   */
    struct sockaddr ddpaddr;	/* Appletalk DDP address */
    struct sockaddr ecaddr;	/* Econet address        */
    int has_ip;
    int has_ipx_bb;
    int has_ipx_sn;
    int has_ipx_e3;
    int has_ipx_e2;
    int has_ax25;
    int has_ddp;
    int has_econet;
    char hwaddr[32];		/* HW address            */
    int statistics_valid;
    struct user_net_device_stats stats;		/* statistics            */
    int keepalive;		/* keepalive value for SLIP */
    int outfill;		/* outfill value for SLIP */
};

class NetworkInterface {
public:
    NetworkInterface(std::unique_ptr<Socket> &socket) :socket_(socket) {};

    int readProc(char *target);

    int readConfig();

    int read();

    int collect(std::function<int (struct interface *, void * , void* ptr)> doit, void *cookie, void *ptr);

    int fetch(struct interface *ife);

private:
    int if_fetch(struct interface *ife);

    int procnetdevVersion(char *buf)
    {
        if (strstr(buf, "compressed"))
            return 3;
        if (strstr(buf, "bytes"))
            return 2;
        return 1;
    }

    char *getName(char *name, char *p);

    int getDevFields(char *bp, struct interface *ife);

    struct interface *ifCacheAdd(char *name);

    int procnetdevVsn = 1;

    int skfd = -1;

    std::map<std::string, std::unique_ptr<interface>> int_list;


    std::unique_ptr<Socket> &socket_;
};
}
