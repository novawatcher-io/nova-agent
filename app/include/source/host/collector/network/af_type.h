//
// Created by zhanglei on 25-2-1.
//

#pragma once

extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
}

namespace App::Source::Host::Collector::Network {
/* This structure defines protocol families and their handlers. */
struct aftype {
    char *name;
    char *title;
    int af;
    int alen;
    char *(*print) (unsigned char *);
    char *(*sprint) (struct sockaddr *, int numeric);
    int (*input) (int type, char *bufp, struct sockaddr *);
    void (*herror) (char *text);
    int (*rprint) (int options);
    int (*rinput) (int typ, int ext, char **argv);

    /* may modify src */
    int (*getmask) (char *src, struct sockaddr * mask, char *name);

    int fd;
    char *flag_file;
};
}