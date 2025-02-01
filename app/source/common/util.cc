//
// Created by zhanglei on 25-2-1.
//
#include "app/include/common/util.h"

extern "C" {
#include <sys/utsname.h>
#include <stdio.h>
}

namespace App::Common {

int kernel_version(void)
{
    struct utsname uts;
    int major, minor, patch;

    if (uname(&uts) < 0)
        return -1;
    if (sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch) != 3)
        return -1;
    return KRELEASE(major, minor, patch);
}

}