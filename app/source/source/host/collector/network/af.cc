//
// Created by zhanglei on 25-2-4.
//
#include "app/include/source/host/collector/network/af.h"

namespace App::Source::Host::Collector::Network {

/* Check our protocol family table for this family. */
struct aftype *get_afntype(int af)
{
    struct aftype **afp;

    if (!sVafinit)
        afinit();

    afp = aftypes;
    while (*afp != NULL) {
        if ((*afp)->af == af)
            return (*afp);
        afp++;
    }
    return (NULL);
}
}