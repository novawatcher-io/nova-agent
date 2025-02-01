//
// Created by zhanglei on 25-2-1.
//
#include "app/include/source/host/collector/network/socket.h"

extern "C" {
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
}
#include "app/include/common/util.h"
#include "app/include/source/host/collector/network/af.h"

namespace App::Source::Host::Collector::Network {

int Socket::open(int family) {
    struct aftype **aft;
    int sfd = -1;
    static int force = -1;

    if (force < 0) {
        force = 0;
        if (Common::kernel_version() < KRELEASE(2, 1, 0))
            force = 1;
        if (access("/proc/net", R_OK))
            force = 1;
    }
    for (aft = aftypes; *aft; aft++) {
        struct aftype *af = *aft;
        int type = SOCK_DGRAM;
        if (af->af == AF_UNSPEC)
            continue;
        if (family && family != af->af)
            continue;
        if (af->fd != -1) {
            sfd = af->fd;
            continue;
        }
        /* Check some /proc file first to not stress kmod */
        if (!family && !force && af->flag_file) {
            if (access(af->flag_file, R_OK))
                continue;
        }
#if HAVE_AFNETROM
        if (af->af == AF_NETROM)
            type = SOCK_SEQPACKET;
#endif
#if HAVE_AFX25
        if (af->af == AF_X25)
            type = SOCK_SEQPACKET;
#endif
        af->fd = socket(af->af, type, 0);
        if (af->fd >= 0)
            sfd = af->fd;
    }
    if (sfd < 0)
        fprintf(stderr, "No usable address families found.\n");
    return sfd;
}
}