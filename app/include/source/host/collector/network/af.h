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
#include "unix.h"
#include "inet.h"

namespace App::Source::Host::Collector::Network {

#define HAVE_AFUNIX 1
#define HAVE_AFINET 1
#define HAVE_AFINET6 0
#define HAVE_AFIPX 1
#define HAVE_AFATALK 1
#define HAVE_AFAX25 1
#define HAVE_AFNETROM 1


extern struct aftype inet6_aftype;
static short sVafinit = 0;
extern struct aftype *get_afntype(int af);

static inline struct aftype unspec_aftype =
{
    "unspec", NULL, /*"UNSPEC", */ AF_UNSPEC, 0,
    UNSPEC_print, UNSPEC_sprint, NULL, NULL,
    NULL,
};

static inline struct aftype *aftypes[] =
{
    &unix_aftype,
    &inet_aftype,
    &inet6_aftype,
    &unspec_aftype,
    NULL
};

static void afinit()
{
    unspec_aftype.title = "UNSPEC";
    unix_aftype.title = "UNIX Domain";
    inet_aftype.title = "DARPA Internet";
    inet6_aftype.title = "IPv6";
    sVafinit = 1;
}




}
