//
// Created by zhanglei on 2025/2/6.
//

#pragma once

#include <memory>
#include "iostat.h"

#define MAX_IOSTAT_NAME_LEN		128

namespace App::Source::Host::Collector::IOStat {
struct io_device {
	char name[MAX_IOSTAT_NAME_LEN];
	/*
	 * 0: Not a whole device (T_PART)
	 * 1: whole device (T_DEV)
	 * 2: whole device and all its partitions to be read (T_PART_DEV)
	 * 3+: group name (T_GROUP) (4 means 1 device in the group, 5 means 2 devices in the group, etc.)
	 */
	int dev_tp;
	/* TRUE if device exists in /proc/diskstats or /sys. Don't apply for groups. */
	bool exist;
	/* major and minor numbers (not set for T_GROUP "devices") */
	int major;
	int minor;
    std::unique_ptr<io_stats> dev_stats[2];
};
}
