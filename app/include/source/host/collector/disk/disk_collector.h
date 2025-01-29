#pragma once
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include "nova_agent_payload/node/v1/info.pb.h"
#include <common/file.h>
#include <cstdint>
#include <map>
#include <memory>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <string>
#include <string_view>
#include <vector>

namespace App::Source::Host::Collector::Disk {
struct DiskInfo {
    std::string device;
    std::string mount_point;
    std::string filesystem;
    std::string vendor;
    int64_t space_total = 0;
    int64_t space_free = 0;
    int64_t inode_free = 0;
    int64_t inode_total = 0;
    int64_t space_used() const {
        return space_total - space_free;
    }
};

struct DiskTime {
    int64_t read_time = 0;
    int64_t write_time = 0;
    int64_t ts = 0; // monotonic ts
};

class DiskCollector {
public:
    DiskCollector(const std::string& company_uuid);

    void GetDiskList(novaagent::node::v1::NodeInfo* request);
    bool GetDiskList(std::vector<DiskInfo>& disks);
    bool GetVendor(std::string_view device_name, std::string& vendor);

    static bool IsTargetFs(std::string_view fs);
    static bool IsPrimaryMount(std::string_view flags);
    novaagent::node::v1::DiskType GetDiskType(std::string_view device_name);

    void GetDiskReadTime(Oltp::MultiValue& values);
    void GetDiskWriteTime(Oltp::MultiValue& values);
    bool ReadProcFSState(std::map<std::string, DiskTime>& disk_stats);

    void Start();

private:
    App::Common::BasicFileReader file_reader_;
    std::vector<std::string> device_names;
    std::string company_uuid_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter_;
    std::vector<std::unique_ptr<Oltp::MetricCollector>> metrics_;
    std::map<std::string, DiskTime> disk_read_stats_;
    std::map<std::string, DiskTime> disk_write_stats_;
};
} // namespace App::Source::Host::Collector::Disk
