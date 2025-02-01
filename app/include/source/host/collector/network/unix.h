//
// Created by zhanglei on 25-2-1.
//

#pragma once
extern "C" {
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
}

#include "app/include/common/util.h"
#include "af_type.h"

namespace App::Source::Host::Collector::Network {

/* Display an UNSPEC address. */
static char *UNSPEC_print(unsigned char *ptr)
{
    static char buff[64];
    char *pos;
    unsigned int i;

    pos = buff;
    for (i = 0; i < sizeof(struct sockaddr); i++) {
        pos += sprintf(pos, "%02X-", (*ptr++ & 0377));
    }
    buff[strlen(buff) - 1] = '\0';
    return (buff);
};

/* Display an UNSPEC socket address. */
static char *UNSPEC_sprint(struct sockaddr *sap, int numeric)
{
    static char buf[64];

    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
        return Common::safe_strncpy(buf, "[NONE SET]", sizeof(buf));
    return (UNSPEC_print((unsigned char *)sap->sa_data));
}

/* Display a UNIX domain address. */
static char *UNIX_print(unsigned char *ptr)
{
    return (char *)(ptr);
}


/* Display a UNIX domain address. */
static char *UNIX_sprint(struct sockaddr *sap, int numeric)
{
    static char buf[64];

    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
        return Common::safe_strncpy(buf, "[NONE SET]", sizeof(buf));
    return (UNIX_print((unsigned char *)sap->sa_data));
}


struct aftype unix_aftype =
{
    "unix", NULL, /*"UNIX Domain", */ AF_UNIX, 0,
    UNIX_print, UNIX_sprint, NULL, NULL,
    NULL, NULL, NULL,
    -1,
    "/proc/net/unix"
};
}