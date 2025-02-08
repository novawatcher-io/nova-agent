//
// Created by zhanglei on 25-2-1.
//

#pragma once

extern "C" {
#include <string.h>
#include <stdlib.h>
}

#include <spdlog/spdlog.h>


namespace App::Common {
#define KRELEASE(maj,min,patch) ((maj) * 10000 + (min)*1000 + (patch))
int kernel_version(void);

/* Like strncpy but make sure the resulting string is always 0 terminated. */
static char *safe_strncpy(char *dst, const char *src, size_t size)
{
  dst[size-1] = '\0';
  return strncpy(dst,src,size-1);
}

static void oom(void)
{
  SPDLOG_ERROR("out of virtual memory\n");
  exit(2);
}


static void *xmalloc(size_t sz)
{
  void *p = calloc(sz, 1);
  if (!p)
    oom();
  return p;
}

static void *xrealloc(void *oldp, size_t sz)
{
  void *p = realloc(oldp, sz);
  if (!p)
    oom();
  return p;
}

/* like strcmp(), but knows about numbers and ':' alias suffix */
int nstrcmp(const char *ap, const char *bp);

/*
 ***************************************************************************
 * Test whether given name is a device or a partition, using sysfs.
 *
 * IN:
 * @sysdev		sysfs location.
 * @name		Device or partition name.
 * @allow_virtual	TRUE if virtual devices are also accepted.
 *			The device is assumed to be virtual if no
 *			/sys/block/<device>/device link exists.
 *
 * RETURNS:
 * TRUE if @name is a device, and FALSE if it's a partition.
 ***************************************************************************
 */
int is_device(char *sysdev, char *name, int allow_virtual);
}
