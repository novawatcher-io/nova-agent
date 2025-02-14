//
// Created by zhanglei on 2025/2/6.
//
#include "app/include/source/host/collector/iostat/handler.h"

extern "C" {
#include <stdio.h>
#include <string.h>
}
#include <cstdlib>
#include "app/include/common/util.h"
#include "app/include/source/host/collector/iostat/iodevice.h"
#include "app/include/source/host/collector/iostat/iostat.h"
#include "app/include/source/host/collector/iostat/systest.h"
#include "app/include/source/host/collector/iostat/common.h"


namespace App::Source::Host::Collector::IOStat {

void Handler::read_diskstats_stat(int curr) {
    if (!alt_dir[0] || USE_ALL_DIR(flags)) {
		/* Read stats from /proc/diskstats */
		read_diskstats_stat_work(curr, DISKSTATS);
	}

    if (alt_dir[0]) {
		char diskstats[MAX_PF_NAME];

		snprintf(diskstats, sizeof(diskstats), "%s/%s", alt_dir, __DISKSTATS);
		diskstats[sizeof(diskstats) - 1] = '\0';
		/* Read stats from an alternate diskstats file */
		read_diskstats_stat_work(curr, diskstats);
	}
}

/*
 ***************************************************************************
 * Read sysfs stats for every whole device from /sys or an alternate
 * location.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 *
 * RETURNS:
 * 0 on success, -1 otherwise.
 ***************************************************************************
 */
int Handler::read_sysfs_all_devices_stat(int curr)
{
	int rc = 0;

	if (!alt_dir[0] || USE_ALL_DIR(flags)) {
		/* Read all whole devices from /sys */
		rc = read_sysfs_all_devices_stat_work(curr, SYSFS_BLOCK);
	}

	if (alt_dir[0]) {
		char sysblock[MAX_PF_NAME];

		snprintf(sysblock, sizeof(sysblock), "%s/%s", alt_dir, __BLOCK);
		sysblock[sizeof(sysblock) - 1] = '\0';
		/* Read stats from an alternate sys location */
		rc = read_sysfs_all_devices_stat_work(curr, sysblock);
	}

	return rc;
}

/*
 ***************************************************************************
 * Read sysfs stats for every whole device. Devices are	saved in the linked
 * list.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @sysblock	__sys/block directory location.
 *
 * RETURNS:
 * 0 on success, -1 otherwise.
 ***************************************************************************
 */
int Handler::read_sysfs_all_devices_stat_work(int curr, char *sysblock)
{
	DIR *dir;
	struct dirent *drd;
	struct io_stats sdev;
	struct io_device *d;
	char dfile[MAX_PF_NAME];

	/* Open __sys/block directory */
	if ((dir = __opendir(sysblock)) == NULL)
		return -1;

	/* Get current entry */
	while ((drd = __readdir(dir)) != NULL) {

		if (!strcmp(drd->d_name, ".") || !strcmp(drd->d_name, ".."))
			continue;
		snprintf(dfile, sizeof(dfile), "%s/%s/%s", sysblock, drd->d_name, S_STAT);

		dfile[sizeof(dfile) - 1] = '\0';

		/* Read current whole device stats */
		if (read_sysfs_file_stat_work(dfile, &sdev) < 0)
			continue;

		d = add_list_device(dev_list, drd->d_name, 0, UKWN_MAJ_NR, 0);
		if (d != NULL) {
			*(d->dev_stats[curr]) = sdev;
		}
	}

	/* Close device directory */
	__closedir(dir);

	return 0;
}


int Handler::read_sysfs_file_stat_work(char *filename, struct io_stats *ios)
{
	FILE *fp;
	struct io_stats sdev;
	int i;
	unsigned int ios_pgr, tot_ticks, rq_ticks, wr_ticks, dc_ticks, fl_ticks;
	unsigned long rd_ios, rd_merges_or_rd_sec, wr_ios, wr_merges;
	unsigned long rd_sec_or_wr_ios, wr_sec, rd_ticks_or_wr_sec;
	unsigned long dc_ios, dc_merges, dc_sec, fl_ios;

	/* Try to read given stat file */
	if ((fp = fopen(filename, "r")) == NULL)
		return -1;

	i = fscanf(fp, "%lu %lu %lu %lu %lu %lu %lu %u %u %u %u %lu %lu %lu %u %lu %u",
		   &rd_ios, &rd_merges_or_rd_sec, &rd_sec_or_wr_ios, &rd_ticks_or_wr_sec,
		   &wr_ios, &wr_merges, &wr_sec, &wr_ticks, &ios_pgr, &tot_ticks, &rq_ticks,
		   &dc_ios, &dc_merges, &dc_sec, &dc_ticks,
		   &fl_ios, &fl_ticks);

	memset(&sdev, 0, sizeof(struct io_stats));

	if (i >= 11) {
		/* Device or partition */
		sdev.rd_ios     = rd_ios;
		sdev.rd_merges  = rd_merges_or_rd_sec;
		sdev.rd_sectors = rd_sec_or_wr_ios;
		sdev.rd_ticks   = (unsigned int) rd_ticks_or_wr_sec;
		sdev.wr_ios     = wr_ios;
		sdev.wr_merges  = wr_merges;
		sdev.wr_sectors = wr_sec;
		sdev.wr_ticks   = wr_ticks;
		sdev.ios_pgr    = ios_pgr;
		sdev.tot_ticks  = tot_ticks;
		sdev.rq_ticks   = rq_ticks;

		if (i >= 15) {
			/* Discard I/O */
			sdev.dc_ios     = dc_ios;
			sdev.dc_merges  = dc_merges;
			sdev.dc_sectors = dc_sec;
			sdev.dc_ticks   = dc_ticks;
		}

		if (i >= 17) {
			/* Flush I/O */
			sdev.fl_ios     = fl_ios;
			sdev.fl_ticks   = fl_ticks;
		}
	}
	else if (i == 4) {
		/* Partition without extended statistics */
		sdev.rd_ios     = rd_ios;
		sdev.rd_sectors = rd_merges_or_rd_sec;
		sdev.wr_ios     = rd_sec_or_wr_ios;
		sdev.wr_sectors = rd_ticks_or_wr_sec;
	}

	*ios = sdev;

	fclose(fp);

	return 0;
}

void Handler::read_sysfs_dlist_stat(int curr) {
	/* Read all whole devices stats if requested ("iostat ALL ...") */
    read_sysfs_all_devices_stat(curr);
}

/*
 ***************************************************************************
 * Read stats from the diskstats file. Only used when "-p ALL" has been
 * entered on the command line.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @diskstats	Path to diskstats file (e.g. "/proc/diskstats").
 ***************************************************************************
 */
void Handler::read_diskstats_stat_work(int curr, char *diskstats)
{
	FILE *fp;
	char line[256], dev_name[MAX_NAME_LEN];
	struct io_device *d;
	struct io_stats sdev;
	int i;
	unsigned int ios_pgr, tot_ticks, rq_ticks, wr_ticks, dc_ticks, fl_ticks;
	unsigned long rd_ios, rd_merges_or_rd_sec, rd_ticks_or_wr_sec, wr_ios;
	unsigned long wr_merges, rd_sec_or_wr_ios, wr_sec;
	unsigned long dc_ios, dc_merges, dc_sec, fl_ios;
	unsigned int major, minor;

	if ((fp = fopen(diskstats, "r")) == NULL)
		return;

	while (fgets(line, sizeof(line), fp) != NULL) {

		memset(&sdev, 0, sizeof(struct io_stats));

		/* major minor name rio rmerge rsect ruse wio wmerge wsect wuse running use aveq dcio dcmerge dcsect dcuse flio fltm */
		i = sscanf(line, "%u %u %s %lu %lu %lu %lu %lu %lu %lu %u %u %u %u %lu %lu %lu %u %lu %u",
			   &major, &minor, dev_name,
			   &rd_ios, &rd_merges_or_rd_sec, &rd_sec_or_wr_ios, &rd_ticks_or_wr_sec,
			   &wr_ios, &wr_merges, &wr_sec, &wr_ticks, &ios_pgr, &tot_ticks, &rq_ticks,
			   &dc_ios, &dc_merges, &dc_sec, &dc_ticks,
			   &fl_ios, &fl_ticks);

		if (i >= 14) {
			sdev.rd_ios     = rd_ios;
			sdev.rd_merges  = rd_merges_or_rd_sec;
			sdev.rd_sectors = rd_sec_or_wr_ios;
			sdev.rd_ticks   = (unsigned int) rd_ticks_or_wr_sec;
			sdev.wr_ios     = wr_ios;
			sdev.wr_merges  = wr_merges;
			sdev.wr_sectors = wr_sec;
			sdev.wr_ticks   = wr_ticks;
			sdev.ios_pgr    = ios_pgr;
			sdev.tot_ticks  = tot_ticks;
			sdev.rq_ticks   = rq_ticks;

			if (i >= 18) {
				/* Discard I/O */
				sdev.dc_ios     = dc_ios;
				sdev.dc_merges  = dc_merges;
				sdev.dc_sectors = dc_sec;
				sdev.dc_ticks   = dc_ticks;
			}

			if (i >= 20) {
				/* Flush I/O */
				sdev.fl_ios     = fl_ios;
				sdev.fl_ticks   = fl_ticks;
			}
		}
		else if (i == 7) {
			/* Partition without extended statistics */
			if (DISPLAY_EXTENDED(flags))
				continue;

			sdev.rd_ios     = rd_ios;
			sdev.rd_sectors = rd_merges_or_rd_sec;
			sdev.wr_ios     = rd_sec_or_wr_ios;
			sdev.wr_sectors = rd_ticks_or_wr_sec;
		}
		else
			/* Unknown entry: Ignore it */
			continue;

		d = add_list_device(dev_list, dev_name, 0, major, minor);
		if (d != NULL) {
			*d->dev_stats[curr] = sdev;
		}
	}
	fclose(fp);
}

/*
 ***************************************************************************
 * Check if a device is present in the list, and add it if requested.
 * Also look for its type (device or partition) and save it.
 *
 * IN:
 * @dlist	Address of pointer on the start of the linked list.
 * @name	Device name.
 * @dtype	T_PART_DEV (=2) if the device and all its partitions should
 *		also be read (option -p used), T_GROUP (=3) if it's a group
 *		name, and 0 otherwise.
 * @major	Major number of the device (set to UKWN_MAJ_NR by caller if
 *		unknown: In this case, major and minor numbers will be
 *		determined here).
 * @minor	Minor number of the device.
 *
 * RETURNS:
 * Pointer on the io_device structure in the list where the device is located
 * (whether it was already in the list or if it has been added).
 * NULL if the device name is too long or if the device doesn't exist and we
 * don't want to add it.
 ***************************************************************************
 */
struct io_device *Handler::add_list_device(std::map<std::string, std::unique_ptr<io_device>>& dlist, char *name, int dtype,
				  int major, int minor)
{
//	struct io_device *d, *ds;
	int i, maj_nr, min_nr;

	if (strnlen(name, MAX_NAME_LEN) == MAX_NAME_LEN)
		/* Device name is too long */
		return NULL;

    auto iter = dlist.find(name);
    if (iter != dlist.end()) {
        /* Device found in list */
        if ((dtype == T_PART_DEV) && (iter->second->dev_tp == T_DEV)) {
            iter->second->dev_tp = dtype;
        }
        iter->second->exist = true;
        return iter->second.get();
    }
    std::unique_ptr<io_device> d = std::make_unique<io_device>();
    auto ret = d.get();

	for (i = 0; i < 2; i++) {
        d->dev_stats[i] = std::make_unique<io_stats>();
	}
    strncpy(d->name, name, sizeof(d->name));

	d->name[MAX_NAME_LEN - 1] = '\0';
	d->exist = true;

	if (dtype == T_GROUP) {
		d->dev_tp = dtype;
	}
	else  {
		int rc = 0;

		if (!alt_dir[0] || USE_ALL_DIR(flags)) {
			rc = Common::is_device(SLASH_SYS, name, true);
		}

		if (alt_dir[0] && (!USE_ALL_DIR(flags) || (USE_ALL_DIR(flags) && !rc))) {
			rc = Common::is_device(alt_dir, name, true);
		}

		if (rc) {
			d->dev_tp = (dtype == T_PART_DEV ? T_PART_DEV : T_DEV);
		}
		else {
			/* This is a partition (T_PART) */
			d->dev_tp = T_PART;
		}

		/* Save major and minor numbers */
		if (major != UKWN_MAJ_NR) {
			d->major = major;
			d->minor = minor;
		}
		else {
			/* Look for device major and minor numbers */
			if (get_major_minor_nr(d->name, &maj_nr, &min_nr) == 0) {
				d->major = maj_nr;
				d->minor = min_nr;
			}
		}
	}

    dlist[name] = std::move(d);
	return ret;
}

/*
 ***************************************************************************
 * Add current device statistics to corresponding group.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @iodev_nr		Number of devices and partitions.
 ***************************************************************************
 */
void Handler::compute_device_groups_stats(int curr, struct io_device *d, struct io_device *g)
{
	if (!DISPLAY_UNFILTERED(flags)) {
		if (!d->dev_stats[curr]->rd_ios &&
		    !d->dev_stats[curr]->wr_ios &&
		    !d->dev_stats[curr]->dc_ios &&
		    !d->dev_stats[curr]->fl_ios)
			return;
	}

	g->dev_stats[curr]->rd_ios     += d->dev_stats[curr]->rd_ios;
	g->dev_stats[curr]->rd_merges  += d->dev_stats[curr]->rd_merges;
	g->dev_stats[curr]->rd_sectors += d->dev_stats[curr]->rd_sectors;
	g->dev_stats[curr]->rd_ticks   += d->dev_stats[curr]->rd_ticks;
	g->dev_stats[curr]->wr_ios     += d->dev_stats[curr]->wr_ios;
	g->dev_stats[curr]->wr_merges  += d->dev_stats[curr]->wr_merges;
	g->dev_stats[curr]->wr_sectors += d->dev_stats[curr]->wr_sectors;
	g->dev_stats[curr]->wr_ticks   += d->dev_stats[curr]->wr_ticks;
	g->dev_stats[curr]->dc_ios     += d->dev_stats[curr]->dc_ios;
	g->dev_stats[curr]->dc_merges  += d->dev_stats[curr]->dc_merges;
	g->dev_stats[curr]->dc_sectors += d->dev_stats[curr]->dc_sectors;
	g->dev_stats[curr]->dc_ticks   += d->dev_stats[curr]->dc_ticks;
	g->dev_stats[curr]->fl_ios     += d->dev_stats[curr]->fl_ios;
	g->dev_stats[curr]->fl_ticks   += d->dev_stats[curr]->fl_ticks;
	g->dev_stats[curr]->ios_pgr    += d->dev_stats[curr]->ios_pgr;
	g->dev_stats[curr]->tot_ticks  += d->dev_stats[curr]->tot_ticks;
	g->dev_stats[curr]->rq_ticks   += d->dev_stats[curr]->rq_ticks;
}

/*
 ***************************************************************************
 * Print everything now (stats and uptime).
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @rectime	Current date and time.
 * @skip	TRUE if nothing should be displayed (option -y). We must
 *		go through write_stats() anyway to compute groups statistics.
 ***************************************************************************
 */
void Handler::write_stats(int curr, struct tm *rectime, int skip) {
    int h, hl = 0, hh = 0, fctr = 1, tab = 4, next = false;
	unsigned long long itv;
	struct io_device *dtmp, *g = NULL;
	char *dev_name;

    /* Calculate time interval in 1/100th of a second */
	itv = Common::get_interval(uptime_cs[!curr], uptime_cs[curr]);

    struct io_stats *ioi, *ioj, iozero;

    memset(&iozero, 0, sizeof(struct io_stats));

    for (auto iter = dev_list.begin(); iter != dev_list.end(); iter++) {
        struct io_device *d = iter->second.get();

        if (d->dev_tp >= T_GROUP) {
            /*
             * This is a new group: Save group position
             * and display previous one.
             */
            memset(g->dev_stats[curr].get(), 0, sizeof(struct io_stats));
        }

        if (!d->exist && (d->dev_tp < T_GROUP))
            /* Current device is non existent (e.g. it has been unregistered from the system */
            continue;

        if ((g != NULL) && (h == hl) && (d->dev_tp < T_GROUP)) {
            /* We are within a group: Increment number of disks in the group */
            (g->dev_tp)++;
            /* Add current device stats to group */
            compute_device_groups_stats(curr, d, g);
        }

        if (DISPLAY_GROUP_TOTAL_ONLY(flags) && (g != NULL) && (d->dev_tp < T_GROUP))
            continue;

        ioi = d->dev_stats[curr].get();
        ioj = d->dev_stats[!curr].get();
        /* Origin (unmerged) flush operations are counted as writes */
        if (!DISPLAY_UNFILTERED(flags)) {
            if (!ioi->rd_ios && !ioi->wr_ios && !ioi->dc_ios && !ioi->fl_ios)
                continue;
        }

        if (DISPLAY_ZERO_OMIT(flags)) {
            if ((ioi->rd_ios == ioj->rd_ios) &&
                (ioi->wr_ios == ioj->wr_ios) &&
                (ioi->dc_ios == ioj->dc_ios) &&
                (ioi->fl_ios == ioj->fl_ios))
                /* No activity: Ignore it */
                continue;
        }

        /* Try to detect if device has been removed then inserted again */
        if (((ioi->rd_ios + ioi->wr_ios + ioi->dc_ios + ioi->fl_ios) <
            (ioj->rd_ios + ioj->wr_ios + ioj->dc_ios + ioj->fl_ios)) &&
            (!ioj->rd_sectors || (ioi->rd_sectors < ioj->rd_sectors)) &&
            (!ioj->wr_sectors || (ioi->wr_sectors < ioj->wr_sectors)) &&
            (!ioj->dc_sectors || (ioi->dc_sectors < ioj->dc_sectors))) {
                ioj = &iozero;
        }

        dev_name = get_device_name(d->major, d->minor, NULL, 0,
                       DISPLAY_DEVMAP_NAME(flags),
                       DISPLAY_PERSIST_NAME_I(flags),
                       false, d->name);

        write_basic_stat(itv, fctr, d, ioi, ioj, tab, dev_name);
    }

}

/*
 ***************************************************************************
 * Write basic stats, read from /proc/diskstats or from sysfs, in plain or
 * JSON format.
 *
 * IN:
 * @itv		Interval of time.
 * @fctr	Conversion factor.
 * @d		Structure containing device description.
 * @ioi		Current sample statistics.
 * @ioj		Previous sample statistics.
 * @tab		Number of tabs to print (JSON format only).
 * @dname	Name to be used for display for current device.
 ***************************************************************************
 */
void Handler::write_basic_stat(unsigned long long itv, int fctr,
		      struct io_device *d, struct io_stats *ioi,
		      struct io_stats *ioj, int tab, char *dname)
{
	unsigned long long rd_sec, wr_sec, dc_sec;

	/* Print stats coming from /sys or /proc/diskstats */
	rd_sec = ioi->rd_sectors - ioj->rd_sectors;
	if ((ioi->rd_sectors < ioj->rd_sectors) && (ioj->rd_sectors <= 0xffffffff)) {
		rd_sec &= 0xffffffff;
	}
	wr_sec = ioi->wr_sectors - ioj->wr_sectors;
	if ((ioi->wr_sectors < ioj->wr_sectors) && (ioj->wr_sectors <= 0xffffffff)) {
		wr_sec &= 0xffffffff;
	}
	dc_sec = ioi->dc_sectors - ioj->dc_sectors;
	if ((ioi->dc_sectors < ioj->dc_sectors) && (ioj->dc_sectors <= 0xffffffff)) {
		dc_sec &= 0xffffffff;
	}


    write_plain_basic_stat(itv, fctr, ioi, ioj, dname,
                   rd_sec, wr_sec, dc_sec);
}

/*
 ***************************************************************************
 * Write basic stats, read from /proc/diskstats or from sysfs, in plain
 * format.
 *
 * IN:
 * @itv		Interval of time.
 * @fctr	Conversion factor.
 * @ioi		Current sample statistics.
 * @ioj		Previous sample statistics.
 * @devname	Current device name.
 * @rd_sec	Number of sectors read.
 * @wr_sec	Number of sectors written.
 * @dc_sec	Number of sectors discarded.
 ***************************************************************************
 */
void Handler::write_plain_basic_stat(unsigned long long itv, int fctr,
			    struct io_stats *ioi, struct io_stats *ioj,
			    char *devname, unsigned long long rd_sec,
			    unsigned long long wr_sec, unsigned long long dc_sec)
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto data = device_stats.find(devname);
    if (data == device_stats.end()) {
        device_stats[devname] = std::make_shared<dev_stats>();
        data = device_stats.find(devname);
    }
	double rsectors, wsectors, dsectors;

	rsectors = S_VALUE(ioj->rd_sectors, ioi->rd_sectors, itv);
	wsectors = S_VALUE(ioj->wr_sectors, ioi->wr_sectors, itv);
	dsectors = S_VALUE(ioj->dc_sectors, ioi->dc_sectors, itv);

	/* tps */
    data->second->tps = S_VALUE(ioj->rd_ios + ioj->wr_ios + ioj->dc_ios,
			  ioi->rd_ios + ioi->wr_ios + ioi->dc_ios, itv);

    data->second->read_bps = rsectors;

    data->second->write_bps = wsectors;

    data->second->dscd_bps = dsectors;

    data->second->read_bytes = (unsigned long long) rd_sec;

    data->second->write_bytes = (unsigned long long) wr_sec;

    data->second->dscd_bytes = (unsigned long long) dc_sec;

}

/*
 ***************************************************************************
 * Initialize stats common structures.
 ***************************************************************************
 */
void Handler::init_stats(void)
{
	int i;

	/* Allocate structures for CPUs "all" and 0 */
	for (i = 0; i < 2; i++) {
		if ((st_cpu[i] = (struct stats_cpu *) malloc(STATS_CPU_SIZE * 2)) == NULL) {
			SPDLOG_ERROR("malloc stats_cpu failed");
			exit(4);
		}
		memset(st_cpu[i], 0, STATS_CPU_SIZE * 2);
	}
}

void Handler::destroy_stats(void) {
	int i;

	/* Allocate structures for CPUs "all" and 0 */
	for (i = 0; i < 2; i++) {
        free(st_cpu[i]);
	}
}


std::map<std::string, std::shared_ptr<dev_stats>> Handler::stat() {
    dev_list.clear();
	int skip = 0;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        device_stats.clear();
    }
    init_stats();
    /* Read system uptime */
    Common::read_uptime(&(uptime_cs[curr]));

    /* Read stats for CPU "all" */
    read_stat_cpu(st_cpu[curr], 1);


    read_sysfs_dlist_stat(curr);

    /* Get time */
    get_xtime(&rectime, 0, 0);
    /* Print results */
    write_stats(curr, &rectime, skip);

    curr ^= 1;

    destroy_stats();

    return device_stats;
}
}