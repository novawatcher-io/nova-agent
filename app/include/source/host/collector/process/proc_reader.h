#pragma once
#include "app/include/source/host/collector/gpu/gpu_reader.h"
#include "common/file.h"
#include "nova_agent_payload/node/v1/info.pb.h"
#include <cstdint>
#include <ctime>
#include <mutex>
#include <string>
#include <string_view>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace App::Source::Host::Collector::Process {
// struct for parse /proc/stat
struct StatInfo {
    uint64_t boot_time = 0;
    struct {
        uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
        uint64_t CPUIdleTime() const {
            return idle + iowait;
        }
        uint64_t CPUNonIdleTime() const {
            return user + nice + system + irq + softirq + steal;
        }
        uint64_t CPUTotalTime() {
            return CPUIdleTime() + CPUNonIdleTime();
        }
    } cpu_time;
};

// struct for parse /proc/meminfo
struct ProcMemoryInfo {
    uint64_t total = 0;
    uint64_t free = 0;
    uint64_t cached = 0;
    uint64_t buffers = 0;

    uint64_t used() const {
        return total - free - cached - buffers;
    }
};

// struct for parse /proc/uptime
struct UptimeInfo {
    double uptime = 0;
    double idle_time = 0;
};

// struct for parse /proc/pid/status
struct ProcessStatus {
    int ppid = 0;
    int uid = 0;
    int gid = 0;
    int fd_size = 0;
    int nonvoluntary_ctxt_switches = 0;
    int voluntary_ctxt_switches = 0;
    int num_threads = 0;
    int64_t swap = 0;
    int kthread = 0;
    std::string name;
    std::string username;
    std::string state;
};

// struct for parse /proc/pid/statm
struct ProcessStatm {
    uint64_t size = 0;
    uint64_t resident = 0;
    uint64_t shared = 0;
    uint64_t text = 0;
    uint64_t lib = 0;
    uint64_t data = 0;
    uint64_t dirty = 0;
};

// struct for parse /proc/pid/stat
struct ProcessStat {
    double running_time = 0;
    uint64_t utime = 0;
    uint64_t stime = 0;
    int nice = 0;
};

struct ProcessIO {
    uint64_t read_bytes = 0;
    uint64_t write_bytes = 0;
};

struct LoadAvgInfo {
    double avg_1 = 0;
    double avg_5 = 0;
    double avg_15 = 0;
    int running = 0;
    int total = 0;
};

// struct for /proc/net/dev
struct NetDevInfo {
    std::string dev_name;
    uint64_t rx_bytes = 0;
    uint64_t rx_packets = 0;
    uint64_t tx_bytes = 0;
    uint64_t tx_packets = 0;
};

struct NetDevList {
    std::vector<NetDevInfo> dev_list;
    uint64_t rx_bytes_total() const {
        uint64_t result = 0;
        for (const auto& dev : dev_list) {
            result += dev.rx_bytes;
        }
        return result;
    }
    uint64_t tx_bytes_total() const {
        uint64_t result = 0;
        for (const auto& dev : dev_list) {
            result += dev.tx_bytes;
        }
        return result;
    }
};

struct ProcessSampleInfo {
    struct timespec io_sample_ts {};
    struct timespec cpu_sample_ts {};

    ::novaagent::node::v1::ProcessInfo* process = nullptr;

    long IODiffInSeconds(const ProcessSampleInfo& other) const {
        return io_sample_ts.tv_sec - other.io_sample_ts.tv_sec;
    }
    long CPUDiffInSeconds(const ProcessSampleInfo& other) const {
        return cpu_sample_ts.tv_sec - other.cpu_sample_ts.tv_sec;
    }
};

static_assert(__linux__, "This code is intended to run on Linux platforms only.");
/**
 * ProcReader is a component that read status of current system.
 * document for /proc filesystem: https://docs.kernel.org/filesystems/proc.html
 */
class ProcReader {
public:
    ProcReader() = default;
    // to make test easy
    explicit ProcReader(App::Common::FileReader* reader) : reader_(reader) {
    }

    void GetMemoryInfo(novaagent::node::v1::VirtualMemoryInfo* memory_info);
    bool ParseProcMemoryInfo(std::string_view content, ProcMemoryInfo& memory_info);

    // external API
    void GetProcList(novaagent::node::v1::ProcessInfoRequest* proc_list);
    bool GetProcDetail(const std::string& content, ProcessSampleInfo& process, ProcessSampleInfo* prev);

    // file content: /proc/pid/status
    static bool ParseProcessStatus(std::string_view content, ::novaagent::node::v1::ProcessInfo* process_info);
    static bool ParseProcessStatus(std::string_view content, ProcessStatus& result);

    // file content: /proc/pid/stat
    bool ParseProcessStat(std::string_view content, ProcessSampleInfo& process, ProcessSampleInfo* prev);
    static bool ParseProcessStat(std::string_view content, ProcessStat& result);

    // file content: /proc/pid/statm
    bool ParseProcessStatm(std::string_view content, ::novaagent::node::v1::ProcessMemoryUsage* memory_info);
    // file content: /proc/pid/cgroup
    static std::string_view ParseProcessCgroup(std::string_view content);

    static bool ParseProcessIO(std::string_view content, ProcessSampleInfo& process, ProcessSampleInfo* prev);
    static bool ParseProcessIO(std::string_view content, ProcessIO& result);

    // file content: /proc/stat
    bool GetProcStat(StatInfo& result);
    static bool ParseStat(std::string_view content, StatInfo& result);

    // file content: /proc/uptime
    static bool ParseUptime(std::string_view content, UptimeInfo& result);

    // file content: /proc/loadavg
    static bool ParseLoadavg(std::string_view content, LoadAvgInfo& result);

    long GetBootTime();
    bool GetUptime(double& uptime);

    std::unordered_map<int, ProcessSampleInfo> GetProcessTable() const;

    // parse /proc/net/dev
    bool GetNetDev(NetDevList& result);
    bool ParseNetDev(std::string_view content, NetDevList& result);

private:
    App::Common::BasicFileReader default_reader_;
    App::Common::FileReader* reader_ = &default_reader_;
    App::Source::Host::Collector::GPU::GPUReader gpu_reader_;

    mutable std::mutex mutex_;
    novaagent::node::v1::ProcessInfoRequest last_;
    std::unordered_map<int, ProcessSampleInfo> process_table_;
    uint64_t total_memory_ = 0;

    inline static long kPageSize = sysconf(_SC_PAGESIZE);
    inline static long kClockTicksPerSecond = sysconf(_SC_CLK_TCK);
};
} // namespace App::Source::Host::Collector::Process
