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
#include <string.h>
}

#include "app/include/source/host/collector/network/pathname.h"
#include "app/include/common/util.h"

namespace App::Source::Host::Collector::Network {

struct interface *NetworkInterface::ifCacheAdd(char *name) {
    struct interface *ife, **nextp, *new1;

    if (!int_list)
        int_last = NULL;

    /* the cache is sorted, so if we hit a smaller if, exit */
    for (ife = int_last; ife; ife = ife->prev) {
        int n = Common::nstrcmp(ife->name, name);
        if (n == 0)
            return ife;
        if (n < 0)
            break;
    }
    Common::xmalloc(sizeof(*(new1)));
    Common::safe_strncpy(new1->name, name, IFNAMSIZ);
    nextp = ife ? &ife->next : &int_list; // keep sorting
    new1->prev = ife;
    new1->next = *nextp;
    if (new1->next)
        new1->next->prev = new1;
    else
        int_last = new1;
    *nextp = new1;
    return new1;
}

int NetworkInterface::readProc(char *target) {
    FILE *fh;
    char buf[512];
    struct interface *ife;
    int err;

    fh = fopen(_PATH_PROCNET_DEV, "r");
    if (!fh) {
        fprintf(stderr, "Warning: cannot open %s (%s). Limited output.\n",
                _PATH_PROCNET_DEV, strerror(errno));
        return -2;
    }
    fgets(buf, sizeof buf, fh);	/* eat line */
    fgets(buf, sizeof buf, fh);

#if 0				/* pretty, but can't cope with missing fields */
    fmt = proc_gen_fmt(_PATH_PROCNET_DEV, 1, fh,
                       "face", "",	/* parsed separately */
                       "bytes", "%lu",
                       "packets", "%lu",
                       "errs", "%lu",
                       "drop", "%lu",
                       "fifo", "%lu",
                       "frame", "%lu",
                       "compressed", "%lu",
                       "multicast", "%lu",
                       "bytes", "%lu",
                       "packets", "%lu",
                       "errs", "%lu",
                       "drop", "%lu",
                       "fifo", "%lu",
                       "colls", "%lu",
                       "carrier", "%lu",
                       "compressed", "%lu",
                       NULL);
    if (!fmt)
        return -1;
#else
    procnetdevVsn = procnetdevVersion(buf);
#endif

    err = 0;
    while (fgets(buf, sizeof buf, fh)) {
        char *s, name[IFNAMSIZ];
        s = getName(name, buf);
        ife = ifCacheAdd(name);
        getDevFields(s, ife);
        ife->statistics_valid = 1;
        if (target && !strcmp(target,name))
            break;
    }
    if (ferror(fh)) {
        perror(_PATH_PROCNET_DEV);
        err = -1;
    }

#if 0
    free(fmt);
#endif
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
        fprintf(stderr, "warning: no inet socket available: %s\n",
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
            perror("SIOCGIFCONF");
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

    if_list_all = 1;

    if (proc_err < 0 && conf_err < 0)
        return -1;
    else
        return 0;
}

int NetworkInterface::all(int (*doit) (struct interface *, void *), void *cookie) {
    struct interface *ife;

    if (!if_list_all && (read() < 0))
        return -1;
    for (ife = int_list; ife; ife = ife->next) {
        int err = doit(ife, cookie);
        if (err)
            return err;
    }
    return 0;
}
}