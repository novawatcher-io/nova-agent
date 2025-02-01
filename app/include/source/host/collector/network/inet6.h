//
// Created by zhanglei on 25-2-1.
//

#pragma once

extern "C" {
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
}
#include "app/include/common/util.h"

namespace App::Source::Host::Collector::Network {
extern int h_errno;		/* some netdb.h versions don't export this */

char * fix_v4_address(char *buf, struct in6_addr *in6)
{
	if (IN6_IS_ADDR_V4MAPPED(in6->s6_addr)) {
			char *s =strchr(buf, '.');
			if (s) {
				while (s > buf && *s != ':')
					--s;
				if (*s == ':') ++s;
				else s = NULL;
			}
			if (s) return s;
	}
	return buf;
}

static int INET6_resolve(char *name, struct sockaddr_in6 *sin6)
{
    struct addrinfo req, *ai;
    int s;

    memset (&req, '\0', sizeof req);
    req.ai_family = AF_INET6;
    if ((s = getaddrinfo(name, NULL, &req, &ai))) {
	fprintf(stderr, "getaddrinfo: %s: %d\n", name, s);
	return -1;
    }
    memcpy(sin6, ai->ai_addr, sizeof(struct sockaddr_in6));

    freeaddrinfo(ai);

    return (0);
}

#ifndef IN6_IS_ADDR_UNSPECIFIED
#define IN6_IS_ADDR_UNSPECIFIED(a) \
        (((__u32 *) (a))[0] == 0 && ((__u32 *) (a))[1] == 0 && \
         ((__u32 *) (a))[2] == 0 && ((__u32 *) (a))[3] == 0)
#endif


static int INET6_rresolve(char *name, struct sockaddr_in6 *sin6, int numeric)
{
    int s;

    /* Grmpf. -FvK */
    if (sin6->sin6_family != AF_INET6) {
#ifdef DEBUG
	fprintf(stderr, _("rresolve: unsupport address family %d !\n"),
		sin6->sin6_family);
#endif
	errno = EAFNOSUPPORT;
	return (-1);
    }
    if (numeric & 0x7FFF) {
	inet_ntop( AF_INET6, &sin6->sin6_addr, name, 80);
	return (0);
    }
    if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
        if (numeric & 0x8000)
	    strcpy(name, "default");
	else
	    strcpy(name, "[::]");
	return (0);
    }

    if ((s = getnameinfo((struct ::sockaddr *) sin6, sizeof(struct sockaddr_in6),
			 name, 255 /* !! */ , NULL, 0, 0))) {
	fputs("getnameinfo failed\n", stderr);
	return -1;
    }
    return (0);
}


static void INET6_reserror(char *text)
{
    herror(text);
}



/* Display an Internet socket address. */
static char *INET6_print(unsigned char *ptr)
{
    static char name[80];

    inet_ntop(AF_INET6, (struct in6_addr *) ptr, name, 80);
	return fix_v4_address(name, (struct in6_addr *)ptr);
}


/* Display an Internet socket address. */
/* dirty! struct sockaddr usually doesn't suffer for inet6 addresses, fst. */
static char *INET6_sprint(struct ::sockaddr *sap, int numeric)
{
    static char buff[128];

    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
	return Common::safe_strncpy(buff, "[NONE SET]", sizeof(buff));
    if (INET6_rresolve(buff, (struct sockaddr_in6 *) sap, numeric) != 0)
	return Common::safe_strncpy(buff, "[UNKNOWN]", sizeof(buff));
    return (fix_v4_address(buff, &((struct sockaddr_in6 *)sap)->sin6_addr));
}


static int INET6_getsock(char *bufp, struct sockaddr *sap)
{
    struct sockaddr_in6 *sin6;
	char *p;

    sin6 = (struct sockaddr_in6 *) sap;
    sin6->sin6_family = AF_INET6;
    sin6->sin6_port = 0;
    sin6->sin6_scope_id = 0;
    sin6->sin6_flowinfo = 0;

    if (inet_pton(AF_INET6, bufp, sin6->sin6_addr.s6_addr) <= 0)
	return (-1);
    p = fix_v4_address(bufp, &sin6->sin6_addr);
    if (p != bufp)
        memcpy(bufp, p, strlen(p)+1);
    return 16;			/* ?;) */
}

static int INET6_input(int type, char *bufp, struct sockaddr *sap)
{
    switch (type) {
    case 1:
	return (INET6_getsock(bufp, sap));
    default:
	return (INET6_resolve(bufp, (struct sockaddr_in6 *) sap));
    }
}




}
