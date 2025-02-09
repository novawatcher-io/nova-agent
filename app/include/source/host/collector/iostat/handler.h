//
// Created by zhanglei on 2025/2/6.
//
#pragma once
#include <cstdint>
#include <ctime>
#include <map>
#include <string>
#include <mutex>
#include <functional>
#include <memory>
#include <absl/base/thread_annotations.h>

#include "rd_stats.h"
#include "iostat.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"

using App::Source::Host::Collector::Oltp::MultiValue;

namespace App::Source::Host::Collector::IOStat {

using DiviceStatsCallabck = std::function<void(std::map<std::string, std::shared_ptr<dev_stats>>&)>;

class Handler {
public:
    // 统计磁盘读写
    std::map<std::string, std::shared_ptr<dev_stats>> stat();


    // 读取所有磁盘的bps
    void loadAllTpsMetric(MultiValue & values) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (device_stats.empty()) {
            return;
        }
        values.resize(device_stats.size() + 1);

        double sum = 0;
        int count = 0;
        for (const auto& data : device_stats) {
            values[count].data = data.second->tps;
            values[count].labels["dev_name"] = data.first;
            sum += data.second->tps;
            count++;
        }
        values[count].data = sum;
        values[count].labels["dev_name"] = "total";
        return;
    }

    // 读取所有磁盘的read/s
    void loadAllReadBpsMetric(MultiValue & values) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (device_stats.empty()) {
            return;
        }
        values.resize(device_stats.size() + 1);

        double sum = 0;
        int count = 0;
        for (const auto& data : device_stats) {
            values[count].data = data.second->read_bps;
            values[count].labels["dev_name"] = data.first;
            sum += data.second->tps;
            count++;
        }
        values[count].data = sum;
        values[count].labels["dev_name"] = "total";
        return;
    }

    // 读取所有磁盘的write/s
    void loadAllWriteBpsMetric(MultiValue & values) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (device_stats.empty()) {
            return;
        }
        values.resize(device_stats.size() + 1);

        double sum = 0;
        int count = 0;
        for (const auto& data : device_stats) {
            values[count].data = data.second->write_bps;
            values[count].labels["dev_name"] = data.first;
            sum += data.second->tps;
            count++;
        }
        values[count].data = sum;
        values[count].labels["dev_name"] = "total";
        return;
    }

    // 读取所有磁盘的dscd/s
    void loadAllDscdBpsMetric(MultiValue & values) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (device_stats.empty()) {
            return;
        }
        values.resize(device_stats.size() + 1);

        double sum = 0;
        int count = 0;
        for (const auto& data : device_stats) {
            values[count].data = data.second->dscd_bps;
            values[count].labels["dev_name"] = data.first;
            sum += data.second->tps;
            count++;
        }
        values[count].data = sum;
        values[count].labels["dev_name"] = "total";
        return;
    }

    // 读取所有磁盘的read bytes
    void loadAllReadBytesMetric(MultiValue & values) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (device_stats.empty()) {
            return;
        }
        values.resize(device_stats.size() + 1);

        double sum = 0;
        int count = 0;
        for (const auto& data : device_stats) {
            values[count].data = double(data.second->read_bytes);
            values[count].labels["dev_name"] = data.first;
            sum += data.second->tps;
            count++;
        }
        values[count].data = sum;
        values[count].labels["dev_name"] = "total";
        return;
    }

    // 读取所有磁盘的write bytes
    void loadAllWriteBytesMetric(MultiValue & values) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (device_stats.empty()) {
            return;
        }
        values.resize(device_stats.size() + 1);

        double sum = 0;
        int count = 0;
        for (const auto& data : device_stats) {
            values[count].data = double(data.second->write_bytes);
            values[count].labels["dev_name"] = data.first;
            sum += data.second->tps;
            count++;
        }
        values[count].data = sum;
        values[count].labels["dev_name"] = "total";
        return;
    }

    // 读取所有磁盘的dscd bytes
    void loadAllDscdBytesMetric(MultiValue & values) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (device_stats.empty()) {
            return;
        }
        values.resize(device_stats.size() + 1);

        double sum = 0;
        int count = 0;
        for (const auto& data : device_stats) {
            values[count].data = double(data.second->dscd_bytes);
            values[count].labels["dev_name"] = data.first;
            sum += data.second->tps;
            count++;
        }
        values[count].data = sum;
        values[count].labels["dev_name"] = "total";
        return;
    }
private:
    void read_diskstats_stat(int curr);

    void read_sysfs_dlist_stat(int curr);

    void read_diskstats_stat_work(int curr, char *diskstats);

    struct io_device *add_list_device(struct io_device **dlist, char *name, int dtype,
				  int major, int minor);

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
    void write_stats(int curr, struct tm *rectime, int skip);

    int read_sysfs_all_devices_stat(int curr);

    int read_sysfs_all_devices_stat_work(int curr, char *sysblock);

    /*
     ***************************************************************************
     * Read sysfs stat for current block device or partition.
     *
     * IN:
     * @filename	File name where stats will be read.
     * @ios		Structure where stats will be saved.
     *
     * OUT:
     * @ios		Structure where stats have been saved.
     *
     * RETURNS:
     * 0 on success, -1 otherwise.
     ***************************************************************************
     */
    int read_sysfs_file_stat_work(char *filename, struct io_stats *ios);

    /*
     ***************************************************************************
     * Add current device statistics to corresponding group.
     *
     * IN:
     * @curr	Index in array for current sample statistics.
     * @iodev_nr		Number of devices and partitions.
     ***************************************************************************
     */
    void compute_device_groups_stats(int curr, struct io_device *d, struct io_device *g);

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
    void write_basic_stat(unsigned long long itv, int fctr,
                  struct io_device *d, struct io_stats *ioi,
                  struct io_stats *ioj, int tab, char *dname);

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
    void write_plain_basic_stat(unsigned long long itv, int fctr,
                    struct io_stats *ioi, struct io_stats *ioj,
                    char *devname, unsigned long long rd_sec,
                    unsigned long long wr_sec, unsigned long long dc_sec);

    /*
     ***************************************************************************
     * Initialize stats common structures.
     ***************************************************************************
     */
    void init_stats(void);

    uint64_t flags = 0;	/* Flag for common options and system state */

    char alt_dir[MAX_FILE_LEN];

    struct stats_cpu *st_cpu[2];

    struct io_device *dev_list = nullptr;

    unsigned long long uptime_cs[2] = {0, 0};

    struct tm rectime;

    int curr = 1;
    // 设备iostat
    std::map<std::string, std::shared_ptr<dev_stats>> device_stats;

    std::mutex mtx_;
};
}
