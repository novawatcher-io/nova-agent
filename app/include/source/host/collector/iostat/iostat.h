//
// Created by zhanglei on 2025/2/6.
//

#pragma once

extern "C" {
#include <time.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
}

namespace App::Source::Host::Collector::IOStat {

#define UKWN_MAJ_NR	0

/*
 * Structures for I/O stats.
 * These are now dynamically allocated.
 */
struct io_stats {
	/* # of sectors read */
	unsigned long rd_sectors	__attribute__ ((aligned (8)));
	/* # of sectors written */
	unsigned long wr_sectors	__attribute__ ((packed));
	/* # of sectors discarded */
	unsigned long dc_sectors	__attribute__ ((packed));
	/* # of read operations issued to the device */
	unsigned long rd_ios		__attribute__ ((packed));
	/* # of read requests merged */
	unsigned long rd_merges		__attribute__ ((packed));
	/* # of write operations issued to the device */
	unsigned long wr_ios		__attribute__ ((packed));
	/* # of write requests merged */
	unsigned long wr_merges		__attribute__ ((packed));
	/* # of discard operations issued to the device */
	unsigned long dc_ios		__attribute__ ((packed));
	/* # of discard requests merged */
	unsigned long dc_merges		__attribute__ ((packed));
	/* # of flush requests issued to the device */
	unsigned long fl_ios		__attribute__ ((packed));
	/* Time of read requests in queue */
	unsigned int  rd_ticks		__attribute__ ((packed));
	/* Time of write requests in queue */
	unsigned int  wr_ticks		__attribute__ ((packed));
	/* Time of discard requests in queue */
	unsigned int  dc_ticks		__attribute__ ((packed));
	/* Time of flush requests in queue */
	unsigned int  fl_ticks		__attribute__ ((packed));
	/* # of I/Os in progress */
	unsigned int  ios_pgr		__attribute__ ((packed));
	/* # of ticks total (for this device) for I/O */
	unsigned int  tot_ticks		__attribute__ ((packed));
	/* # of ticks requests spent in queue */
	unsigned int  rq_ticks		__attribute__ ((packed));
};

/*
 * Structures for I/O stats.
 * These are now dynamically allocated.
 */
class dev_stats {
public:
	/* # tps */
	double tps;
	// 每一读取的数据量
	double read_bps;
    // 每一秒写入的数据量
    double write_bps;
	// 设备每秒完成的丢弃请求数（合并后）。
	double dscd_bps;
    // 读入的block总数
    unsigned long long read_bytes;
    // 写入的block总数.
    unsigned long long write_bytes;
    // 设备完成的丢弃请求数（合并后）
    unsigned long long dscd_bytes;
};

enum {
	T_PART		= 0,
	T_DEV		= 1,
	T_PART_DEV	= 2,
	T_GROUP		= 3
};

/* I_: iostat - D_: Display - F_: Flag */
#define I_D_CPU			0x000001
#define I_D_DISK		0x000002
#define I_D_TIMESTAMP		0x000004
#define I_D_EXTENDED		0x000008
#define I_D_EVERYTHING		0x000010
#define I_D_KILOBYTES		0x000020
#define I_D_ALL_DIR		0x000040
/* Unused			0x000080 */
#define I_D_UNFILTERED		0x000100
#define I_D_MEGABYTES		0x000200
#define I_D_ALL_DEVICES		0x000400
#define I_F_GROUP_DEFINED	0x000800
#define I_D_PRETTY		0x001000
#define I_D_PERSIST_NAME	0x002000
#define I_D_OMIT_SINCE_BOOT	0x004000
/* Unused			0x008000 */
#define I_D_DEVMAP_NAME		0x010000
/* Unused			0x020000 */
#define I_D_GROUP_TOTAL_ONLY	0x040000
#define I_D_ZERO_OMIT		0x080000
#define I_D_UNIT		0x100000
#define I_D_SHORT_OUTPUT	0x200000
#define I_D_COMPACT		0x400000

#define DISPLAY_CPU(m)			(((m) & I_D_CPU)              == I_D_CPU)
#define DISPLAY_DISK(m)			(((m) & I_D_DISK)             == I_D_DISK)
#define DISPLAY_TIMESTAMP(m)		(((m) & I_D_TIMESTAMP)        == I_D_TIMESTAMP)
#define DISPLAY_EXTENDED(m)		(((m) & I_D_EXTENDED)         == I_D_EXTENDED)
#define DISPLAY_EVERYTHING(m)		(((m) & I_D_EVERYTHING)       == I_D_EVERYTHING)
#define DISPLAY_KILOBYTES(m)		(((m) & I_D_KILOBYTES)        == I_D_KILOBYTES)
#define DISPLAY_MEGABYTES(m)		(((m) & I_D_MEGABYTES)        == I_D_MEGABYTES)
#define DISPLAY_UNFILTERED(m)		(((m) & I_D_UNFILTERED)       == I_D_UNFILTERED)
#define DISPLAY_ALL_DEVICES(m)		(((m) & I_D_ALL_DEVICES)      == I_D_ALL_DEVICES)
#define GROUP_DEFINED(m)		(((m) & I_F_GROUP_DEFINED)    == I_F_GROUP_DEFINED)
#define DISPLAY_PRETTY(m)		(((m) & I_D_PRETTY)           == I_D_PRETTY)
#define DISPLAY_PERSIST_NAME_I(m)	(((m) & I_D_PERSIST_NAME)     == I_D_PERSIST_NAME)
#define DISPLAY_OMIT_SINCE_BOOT(m)	(((m) & I_D_OMIT_SINCE_BOOT)  == I_D_OMIT_SINCE_BOOT)
#define DISPLAY_DEVMAP_NAME(m)		(((m) & I_D_DEVMAP_NAME)      == I_D_DEVMAP_NAME)
#define DISPLAY_GROUP_TOTAL_ONLY(m)	(((m) & I_D_GROUP_TOTAL_ONLY) == I_D_GROUP_TOTAL_ONLY)
#define DISPLAY_ZERO_OMIT(m)		(((m) & I_D_ZERO_OMIT)        == I_D_ZERO_OMIT)
#define DISPLAY_UNIT(m)			(((m) & I_D_UNIT)	      == I_D_UNIT)
#define DISPLAY_SHORT_OUTPUT(m)		(((m) & I_D_SHORT_OUTPUT)     == I_D_SHORT_OUTPUT)
#define USE_ALL_DIR(m)			(((m) & I_D_ALL_DIR)          == I_D_ALL_DIR)
#define DISPLAY_COMPACT(m)		(((m) & I_D_COMPACT)          == I_D_COMPACT)

/*
 ***************************************************************************
 * Get device major and minor numbers.
 *
 * IN:
 * @filename	Name of the device ("sda", "/dev/sdb1"...)
 *
 * OUT:
 * @major	Major number of the device.
 * @minor	Minor number of the device.
 *
 * RETURNS:
 * 0 on success, and -1 otherwise.
 ***************************************************************************
 */
static int get_major_minor_nr(char filename[], int *major_, int *minor_)
{
	struct stat statbuf;
	char *bang;
	char dfile[MAX_PF_NAME];

	snprintf(dfile, sizeof(dfile), "%s%s", filename[0] == '/' ? "" : SLASH_DEV, filename);
	dfile[sizeof(dfile) - 1] = '\0';

	while ((bang = strchr(dfile, '!'))) {
		/*
		 * Some devices may have had a slash replaced with a bang character (eg. cciss!c0d0...)
		 * Restore their original names so that they can be found in /dev directory.
		 */
		*bang = '/';
	}

	if (stat(dfile, &statbuf) < 0)
		return -1;

	*major_ = major(statbuf.st_rdev);
	*minor_ = minor(statbuf.st_rdev);

	return 0;
}

/*
 ***************************************************************************
 * Get device mapper name (e.g. "dm-0") from its registered name (e.g.
 * "virtualhd-home").
 *
 * IN:
 * @name	Registered name of the device (e.g. "virtualhd-home").
 *
 * RETURNS:
 * Name of the device mapper name (e.g. "dm-0").
 ***************************************************************************
 */
static char *get_dm_name_from_registered_name(char *registered_name)
{
	int n;
	char filen[PATH_MAX];
	char target[PATH_MAX];

	/*
	 * The registered device name is a symlink pointing at its device mapper name
	 * in the /dev/mapper directory.
	 */
	n = snprintf(filen, sizeof(filen), "%s/%s", DEVMAP_DIR, registered_name);
	if ((n >= sizeof(filen)) || access(filen, F_OK)) {
		return (NULL);
	}

	/* Read symlink */
	n = readlink(filen, target, PATH_MAX);
	if ((n <= 0) || (n >= PATH_MAX))
		return (NULL);

	target[n] = '\0';

	/* ... and get device mapper name it points at */
	return basename(target);
}
}
