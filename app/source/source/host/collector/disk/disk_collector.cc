#include "app/include/source/host/collector/disk/disk_collector.h"
#include "absl/strings/numbers.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include "common/file.h"
#include "fmt/format.h"
#include "node/v1/info.pb.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include <absl/strings/str_split.h>
#include <algorithm>
#include <bits/chrono.h>
#include <cassert>
#include <cstddef>
#include <ctype.h>
#include <errno.h>
#include <map>
#include <memory>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/provider.h>
#include <spdlog/spdlog.h>
#include <string.h>
#include <string>
#include <string_view>
#include <sys/statvfs.h>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

using App::Source::Host::Collector::Oltp::MetricCollector;
using App::Source::Host::Collector::Oltp::MetricData;
using App::Source::Host::Collector::Oltp::MultiValue;
using App::Source::Host::Collector::Oltp::SingleValue;
using deepagent::node::v1::DiskType;

namespace App::Source::Host::Collector::Disk {
void DiskCollector::GetDiskList(deepagent::node::v1::NodeInfo* request) {
    assert(request);
    std::vector<DiskInfo> disks;
    if (!GetDiskList(disks)) {
        SPDLOG_ERROR("GetDiskList failed");
        return;
    }
    for (auto& disk : disks) {
        auto* target = request->add_disk_infos();
        target->set_space_total(disk.space_total);
        target->set_space_free(disk.space_free);
        target->set_space_used(disk.space_total - disk.space_free);
        if (disk.space_total > 0) {
            target->set_space_usage(static_cast<double>(target->space_used()) / target->space_total());
        }
        target->set_inode_total(disk.inode_total);
        target->set_inode_free(disk.inode_free);
        target->set_inode_used(disk.inode_total - disk.inode_free);
        if (disk.inode_total > 0) {
            target->set_inode_usage(static_cast<double>(target->inode_used()) / disk.inode_total);
        }

        target->set_device(disk.device);
        target->set_mount_point(disk.mount_point);
        target->set_filesystem(disk.filesystem);
        target->set_vendor(disk.vendor);
    }
}

bool DiskCollector::GetDiskList(std::vector<DiskInfo>& disks) {
    std::string content;
    if (!file_reader_.ReadFile("/proc/mounts", &content)) {
        SPDLOG_ERROR("could not read file: /proc/mounts");
        return false;
    }

    std::vector<std::string_view> const lines = absl::StrSplit(content, '\n');
    constexpr size_t kExpectFields = 6;
    for (auto& line : lines) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string_view> fields = absl::StrSplit(line, ' ');
        if (fields.size() != kExpectFields) {
            SPDLOG_ERROR("could not parse line: {}|", line);
            continue;
        }

        auto& device = fields[0];
        std::string mount_point(fields[1]);
        auto& filesystem = fields[2];
        auto& flags = fields[3];

        if (device.find("/dev/") != 0) {
            continue;
        }
        // if (!IsTargetFs(filesystem)) {
        //     SPDLOG_INFO("Skip filesystem: {}", filesystem);
        //     continue;
        // }
        // if (!IsPrimaryMount(flags)) {
        //     SPDLOG_INFO("not primary mount: {}", filesystem);
        //     continue;
        // }
        struct statvfs stat {};
        if (statvfs(mount_point.c_str(), &stat) != 0) {
            SPDLOG_ERROR("statvfs failed, mount point: {}, reason: {}", mount_point.c_str(), strerror(errno));
            continue;
        }
        DiskInfo disk_info;
        disk_info.device = device;
        disk_info.mount_point = mount_point;
        disk_info.space_total = stat.f_blocks * stat.f_frsize;
        disk_info.space_free = stat.f_bfree * stat.f_frsize;
        disk_info.inode_total = stat.f_files;
        disk_info.inode_free = stat.f_ffree;
        GetVendor(device.substr(5), disk_info.vendor);

        disks.push_back(disk_info);
    }
    return true;
}

bool DiskCollector::IsTargetFs(std::string_view fs) {
    std::unordered_set<std::string> set = {
        "ext2", "ext3", "ext4", "xfs", "btrfs", "reiserfs", "jfs", "zfs",
    };
    std::string fs_lower(fs);
    std::transform(fs_lower.begin(), fs_lower.end(), fs_lower.begin(), ::tolower);
    return set.find(fs_lower) != set.end();
}

bool DiskCollector::GetVendor(std::string_view device_name, std::string& vendor) {
    std::string vendor_path = fmt::format("/sys/class/block/{}/device/vendor", device_name);
    std::string content;
    if (!file_reader_.ReadFile(vendor_path, &content)) {
        return false;
    }
    content.erase(std::remove_if(content.begin(), content.end(), ::isspace), content.end());
    vendor = content;
    return true;
}

DiskType DiskCollector::GetDiskType(std::string_view device_name) {
    std::string file = fmt::format("/sys/class/block/{}/queue/rotational", device_name);
    std::string content;
    if (!file_reader_.ReadFile(file, &content)) {
        return DiskType::Unkonwn;
    }

    int rotational = 1; // Default to HDD (rotational) if we can't read the value
    if (absl::SimpleAtoi(content, &rotational)) {
        // If 0, it's an SSD (non-rotational)
        if (rotational == 0) {
            return DiskType::SSD;
        }
    }
    return DiskType::HDD;
}

bool DiskCollector::IsPrimaryMount(std::string_view flags) {
    // ro,noexec,noatime
    std::vector<std::string_view> fields = absl::StrSplit(flags, ',');
    for (auto& attr : fields) {
        if (attr == "rw") {
            return true;
        }
    }
    return false;
}

DiskCollector::DiskCollector(const std::string& company_uuid) : company_uuid_(company_uuid) {
    auto provider = opentelemetry::metrics::Provider::GetMeterProvider();
    meter_ = provider->GetMeter("disk", "1.2.0");

    metrics_.emplace_back(
        MetricCollector::Create<int64_t>(meter_, company_uuid_, "system_disk_used", [&](MultiValue& values) {
            std::vector<DiskInfo> disks;
            GetDiskList(disks);
            for (auto& disk : disks) {
                MetricData metric;
                metric.labels["device"] = disk.device;
                metric.data = disk.space_used();
                values.push_back(metric);
            }
        }));

    metrics_.emplace_back(MetricCollector::Create<double>(meter_, company_uuid_, "system_disk_read_time_pct",
                                                          [&](MultiValue& values) { GetDiskReadTime(values); }));

    metrics_.emplace_back(MetricCollector::Create<double>(meter_, company_uuid_, "system_disk_write_time_pct",
                                                          [&](MultiValue& values) { GetDiskWriteTime(values); }));
}

void DiskCollector::GetDiskReadTime(Oltp::MultiValue& values) {
    std::map<std::string, DiskTime> disk_stats;
    ReadProcFSState(disk_stats);
    for (auto& [dev, usage] : disk_stats) {
        auto it = disk_read_stats_.find(dev);
        if (it == disk_read_stats_.end()) {
            continue;
        }
        auto d_read = usage.read_time - it->second.read_time;
        auto dtime = usage.ts - it->second.ts;
        if (dtime > 0) {
            MetricData metric;
            metric.data = static_cast<double>(d_read) / dtime;
            metric.labels["device"] = dev;
            values.push_back(metric);
        }
    }

    disk_read_stats_.swap(disk_stats);
}

void DiskCollector::GetDiskWriteTime(Oltp::MultiValue& values) {
    std::map<std::string, DiskTime> disk_stats;
    ReadProcFSState(disk_stats);
    for (auto& [dev, usage] : disk_stats) {
        auto it = disk_write_stats_.find(dev);
        if (it == disk_write_stats_.end()) {
            continue;
        }
        auto d_read = usage.write_time - it->second.write_time;
        auto dtime = usage.ts - it->second.ts;
        if (dtime > 0) {
            MetricData metric;
            metric.data = static_cast<double>(d_read) / dtime;
            metric.labels["device"] = dev;
            values.push_back(metric);
        }
    }

    disk_write_stats_.swap(disk_stats);
}

bool DiskCollector::ReadProcFSState(std::map<std::string, DiskTime>& disk_stats) {
    std::string content;
    if (!file_reader_.ReadFile("/proc/diskstats", &content)) {
        return false;
    }

    std::vector<std::string_view> lines = absl::StrSplit(content, '\n');
    for (auto& line : lines) {
        std::vector<std::string_view> fields = absl::StrSplit(line, ' ');
        if (fields[2].substr(0, 4) == "loop") {
            continue;
        }
        std::string name(fields[2]);
        DiskTime stat;
        absl::SimpleAtoi(fields[6], &stat.read_time);
        absl::SimpleAtoi(fields[10], &stat.write_time);
        stat.ts = std::chrono::steady_clock::now().time_since_epoch().count();
        disk_stats[name] = stat;
    }
    return true;
}

void DiskCollector::Start() {
    for (auto& metric : metrics_) {
        metric->Start();
    }
}

} // namespace App::Source::Host::Collector::Disk
