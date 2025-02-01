//
// Created by zhanglei on 25-2-1.
//

#pragma once

extern "C" {
#include <string.h>
}

namespace App::Common {
#define KRELEASE(maj,min,patch) ((maj) * 10000 + (min)*1000 + (patch))
int kernel_version(void);

/* Like strncpy but make sure the resulting string is always 0 terminated. */
static char *safe_strncpy(char *dst, const char *src, size_t size)
{
  dst[size-1] = '\0';
  return strncpy(dst,src,size-1);
}



}
