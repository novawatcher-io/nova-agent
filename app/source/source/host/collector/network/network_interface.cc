//
// Created by zhanglei on 25-2-3.
//
#include "app/include/source/host/collector/network/network_interface.h"

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <net/if_arp.h>
#include <linux/if_slip.h>
#include <string.h>
}

#include "app/include/source/host/collector/network/pathname.h"
#include "app/include/source/host/collector/network/ipx.h"
#include "app/include/common/util.h"

namespace App::Source::Host::Collector::Network {

static int ipx_getaddr(int sock, int ft, struct ifreq *ifr)
{
    ((struct sockaddr_ipx *) &ifr->ifr_addr)->sipx_type = ft;
    return ioctl(sock, SIOCGIFADDR, ifr);
}

struct interface *NetworkInterface::ifCacheAdd(char *name) {
    std::unique_ptr<interface> ife;
    struct interface* ifeItr = nullptr;
    struct interface* next;

    auto iter = int_list.find(name);
    if (iter != int_list.end()) {
        return iter->second.get();
    }
    std::unique_ptr<interface> newInterface = std::make_unique<interface>();
    auto newInterfacePtr = newInterface.get();
    newInterface->name = name;
    int_list[name] = std::move(newInterface);
    return newInterfacePtr;
}

int NetworkInterface::readProc(char *target) {
    FILE *fh;
    char buf[512];
    struct interface *ife = nullptr;
    int err;

    fh = fopen(_PATH_PROCNET_DEV, "r");
    if (!fh) {
        SPDLOG_ERROR("Warning: cannot open %s (%s). Limited output.\n",
                _PATH_PROCNET_DEV, strerror(errno));
        return -2;
    }
    fgets(buf, sizeof buf, fh);	/* eat line */
    fgets(buf, sizeof buf, fh);


    procnetdevVsn = procnetdevVersion(buf);

    err = 0;
    while (fgets(buf, sizeof buf, fh)) {
        char *s, name[IFNAMSIZ];
        s = getName(name, buf);
        ife = ifCacheAdd(name);
        getDevFields(s, ife);
        ife->statistics_valid = 1;
        if (target && !strcmp(target, name))
            break;
    }
    if (ferror(fh)) {
        perror(_PATH_PROCNET_DEV);
        err = -1;
    }

    fclose(fh);
    return err;
}

int NetworkInterface::readConfig() {
    int numreqs = 30;
    struct ifconf ifc;
    struct ifreq *ifr;
    int n, err = -1;
    int skfd;

    /* SIOCGIFCONF currently seems to only work properly on AF_INET sockets
       (as of 2.1.128) */
    skfd = socket_->af(AF_INET);
    if (skfd < 0) {
        SPDLOG_ERROR("warning: no inet socket available: %s\n",
                strerror(errno));
        /* Try to soldier on with whatever socket we can get hold of.  */
        skfd = socket_->open(0);
        if (skfd < 0)
            return -1;
    }

    ifc.ifc_buf = NULL;
    for (;;) {
        ifc.ifc_len = sizeof(struct ifreq) * numreqs;
        ifc.ifc_buf = (char *)Common::xrealloc(ifc.ifc_buf, ifc.ifc_len);

        if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
            SPDLOG_ERROR("SIOCGIFCONF");
            goto out;
        }
        if (ifc.ifc_len == sizeof(struct ifreq) * numreqs) {
            /* assume it overflowed and try again */
            numreqs *= 2;
            continue;
        }
        break;
    }

    ifr = ifc.ifc_req;
    for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) {
        ifCacheAdd(ifr->ifr_name);
        ifr++;
    }
    err = 0;

    out:
        free(ifc.ifc_buf);
    return err;
}

char *NetworkInterface::getName(char *name, char *p) {
    while (isspace(*p))
        p++;
    while (*p) {
        if (isspace(*p))
            break;
        if (*p == ':') {	/* could be an alias */
            char *dot = p++;
            while (*p && isdigit(*p)) p++;
            if (*p == ':') {
                /* Yes it is, backup and copy it. */
                p = dot;
                *name++ = *p++;
                while (*p && isdigit(*p)) {
                    *name++ = *p++;
                }
            } else {
                /* No, it isn't */
                p = dot;
            }
            p++;
            break;
        }
        *name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

int NetworkInterface::getDevFields(char *bp, struct interface *ife)
{
    switch (procnetdevVsn) {
    case 3:
        sscanf(bp,
        "%Lu %Lu %lu %lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu %lu",
               &ife->stats.rx_bytes,
               &ife->stats.rx_packets,
               &ife->stats.rx_errors,
               &ife->stats.rx_dropped,
               &ife->stats.rx_fifo_errors,
               &ife->stats.rx_frame_errors,
               &ife->stats.rx_compressed,
               &ife->stats.rx_multicast,

               &ife->stats.tx_bytes,
               &ife->stats.tx_packets,
               &ife->stats.tx_errors,
               &ife->stats.tx_dropped,
               &ife->stats.tx_fifo_errors,
               &ife->stats.collisions,
               &ife->stats.tx_carrier_errors,
               &ife->stats.tx_compressed);
        break;
    case 2:
        sscanf(bp, "%Lu %Lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu",
               &ife->stats.rx_bytes,
               &ife->stats.rx_packets,
               &ife->stats.rx_errors,
               &ife->stats.rx_dropped,
               &ife->stats.rx_fifo_errors,
               &ife->stats.rx_frame_errors,

               &ife->stats.tx_bytes,
               &ife->stats.tx_packets,
               &ife->stats.tx_errors,
               &ife->stats.tx_dropped,
               &ife->stats.tx_fifo_errors,
               &ife->stats.collisions,
               &ife->stats.tx_carrier_errors);
        ife->stats.rx_multicast = 0;
        break;
    case 1:
        sscanf(bp, "%Lu %lu %lu %lu %lu %Lu %lu %lu %lu %lu %lu",
               &ife->stats.rx_packets,
               &ife->stats.rx_errors,
               &ife->stats.rx_dropped,
               &ife->stats.rx_fifo_errors,
               &ife->stats.rx_frame_errors,

               &ife->stats.tx_packets,
               &ife->stats.tx_errors,
               &ife->stats.tx_dropped,
               &ife->stats.tx_fifo_errors,
               &ife->stats.collisions,
               &ife->stats.tx_carrier_errors);
        ife->stats.rx_bytes = 0;
        ife->stats.tx_bytes = 0;
        ife->stats.rx_multicast = 0;
        break;
    }
    return 0;
}

int NetworkInterface::read() {
    /* caller will/should check not to call this too often
     *   (i.e. only if if_list_all == 0
     */
    int proc_err, conf_err;

    proc_err = readProc(NULL);
    conf_err = readConfig();

    if (proc_err < 0 && conf_err < 0)
        return -1;
    else
        return 0;
}

int NetworkInterface::collect(std::function<int (struct interface *, void * , void* ptr)> doit, void *cookie, void* ptr) {
    if (skfd == -1) {
        skfd = socket_->open(0);
        if (skfd == -1) {
            return -1;
        }
    }

    if (read() < 0) {
        return -1;
    }
    for (auto iter = int_list.begin(); iter != int_list.end(); iter++) {
        int err = doit(iter->second.get(), cookie, ptr);
        if (err) {
            return err;
        }
    }
    return 0;
}

/* Fetch the interface configuration from the kernel. */
int NetworkInterface::if_fetch(struct interface *ife)
{
    struct ifreq ifr;
    int fd;
    std::string ifname = ife->name;

    strcpy(ifr.ifr_name, ifname.c_str());
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
        SPDLOG_ERROR("get SIOCGIFFLAGS failed: {}", strerror(errno));
        return (-1);
    }
    ife->flags = ifr.ifr_flags;

    strcpy(ifr.ifr_name, ifname.c_str());
    if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
	memset(ife->hwaddr, 0, 32);
    else
	memcpy(ife->hwaddr, ifr.ifr_hwaddr.sa_data, 8);

    ife->type = ifr.ifr_hwaddr.sa_family;

    strcpy(ifr.ifr_name, ifname.c_str());
    if (ioctl(skfd, SIOCGIFMETRIC, &ifr) < 0)
	ife->metric = 0;
    else
	ife->metric = ifr.ifr_metric;

    strcpy(ifr.ifr_name, ifname.c_str());
    if (ioctl(skfd, SIOCGIFMTU, &ifr) < 0)
	ife->mtu = 0;
    else
	ife->mtu = ifr.ifr_mtu;

    if (ife->type == ARPHRD_SLIP || ife->type == ARPHRD_CSLIP ||
	ife->type == ARPHRD_SLIP6 || ife->type == ARPHRD_CSLIP6 ||
	ife->type == ARPHRD_ADAPT) {

	strcpy(ifr.ifr_name, ifname.c_str());
	if (ioctl(skfd, SIOCGOUTFILL, &ifr) < 0)
	    ife->outfill = 0;
	else
	    ife->outfill = (unsigned long) ifr.ifr_data;


	strcpy(ifr.ifr_name, ifname.c_str());
	if (ioctl(skfd, SIOCGKEEPALIVE, &ifr) < 0)
	    ife->keepalive = 0;
	else
	    ife->keepalive = (unsigned long) ifr.ifr_data;

    }

    strcpy(ifr.ifr_name, ifname.c_str());
    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0)
	memset(&ife->map, 0, sizeof(struct ifmap));
    else
	memcpy(&ife->map, &ifr.ifr_map, sizeof(struct ifmap));

    strcpy(ifr.ifr_name, ifname.c_str());
    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0)
	memset(&ife->map, 0, sizeof(struct ifmap));
    else
	ife->map = ifr.ifr_map;


    strcpy(ifr.ifr_name, ifname.c_str());
    if (ioctl(skfd, SIOCGIFTXQLEN, &ifr) < 0)
	ife->tx_queue_len = -1;	/* unknown value */
    else
	ife->tx_queue_len = ifr.ifr_qlen;

    ife->tx_queue_len = -1;	/* unknown value */


    /* IPv4 address? */
    fd = socket_->af(AF_INET);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname.c_str());
	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
	    ife->has_ip = 1;
	    ife->addr = ifr.ifr_addr;
	    strcpy(ifr.ifr_name, ifname.c_str());
	    if (ioctl(fd, SIOCGIFDSTADDR, &ifr) < 0)
	        memset(&ife->dstaddr, 0, sizeof(struct sockaddr));
	    else
	        ife->dstaddr = ifr.ifr_dstaddr;

	    strcpy(ifr.ifr_name, ifname.c_str());
	    if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0)
	        memset(&ife->broadaddr, 0, sizeof(struct sockaddr));
	    else
		ife->broadaddr = ifr.ifr_broadaddr;

	    strcpy(ifr.ifr_name, ifname.c_str());
	    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)
		memset(&ife->netmask, 0, sizeof(struct sockaddr));
	    else
		ife->netmask = ifr.ifr_netmask;
	} else
	    memset(&ife->addr, 0, sizeof(struct sockaddr));
    }

    /* DDP address maybe ? */
    fd = socket_->af(AF_APPLETALK);
    if (fd >= 0) {
        strcpy(ifr.ifr_name, ifname.c_str());
        if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
            ife->ddpaddr = ifr.ifr_addr;
            ife->has_ddp = 1;
        }
    }


    /* Look for IPX addresses with all framing types */
    fd = socket_->af(AF_IPX);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname.c_str());
	if (!ipx_getaddr(fd, IPX_FRAME_ETHERII, &ifr)) {
	    ife->has_ipx_bb = 1;
	    ife->ipxaddr_bb = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname.c_str());
	if (!ipx_getaddr(fd, IPX_FRAME_SNAP, &ifr)) {
	    ife->has_ipx_sn = 1;
	    ife->ipxaddr_sn = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname.c_str());
	if (!ipx_getaddr(fd, IPX_FRAME_8023, &ifr)) {
	    ife->has_ipx_e3 = 1;
	    ife->ipxaddr_e3 = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname.c_str());
        if (!ipx_getaddr(fd, IPX_FRAME_8022, &ifr)) {
            ife->has_ipx_e2 = 1;
            ife->ipxaddr_e2 = ifr.ifr_addr;
        }
    }


    /* Econet address maybe? */
    fd = socket_->af(AF_ECONET);
    if (fd >= 0) {
        strcpy(ifr.ifr_name, ifname.c_str());
        if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
            ife->ecaddr = ifr.ifr_addr;
            ife->has_econet = 1;
        }
    }

    return 0;
}

int NetworkInterface::fetch(struct interface *ife)
{
    if (if_fetch(ife) < 0) {
        char *errmsg;
        if (errno == ENODEV) {
            /* Give better error message for this case. */
            errmsg = "Device not found";
        } else {
            errmsg = strerror(errno);
        }
        SPDLOG_ERROR("{}: error fetching interface information: {}\n",
              ife->name, errmsg);
        return -1;
    }
    return 0;
}
}