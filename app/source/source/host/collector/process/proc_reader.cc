#include "app/include/source/host/collector/process/proc_reader.h"
#include "common/parser.h"
#include "node/v1/info.pb.h"
#include "source/host/collector/gpu/gpu_reader.h"
#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <absl/strings/string_view.h>
#include <array>
#include <filesystem>
#include <functional>
#include <map>
#include <math.h>
#include <mutex>
#include <pwd.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

using ::novaagent::node::v1::ProcessInfo;
using ::novaagent::node::v1::ProcessState;
using director_iterator = std::filesystem::directory_iterator;
using App::Common::ConvertStr2Number;
using App::Common::ParseKeyVal;
using App::Common::ParseStr2Number;
using App::Common::ReadKeyVal;
using App::Source::Host::Collector::GPU::GPUProcessUsage;

namespace App::Source::Host::Collector::Process {

ProcessState GetProcessState(const std::string& state) {
    std::map<char, ProcessState> process_states = {
        {'R', ProcessState::Running}, {'S', ProcessState::Sleeping}, {'D', ProcessState::Uninterruptible},
        {'T', ProcessState::Traced},  {'Z', ProcessState::Zombie},
    };
    if (state.empty()) {
        return ProcessState::Unknown;
    }
    auto it = process_states.find(state[0]);
    if (it == process_states.end()) {
        return ProcessState::Unknown;
    }
    return it->second;
}

std::string GetUsernameFromUID(uid_t uid) {
    struct passwd pwd {};
    struct passwd* result = nullptr;
    std::array<char, 1024> buffer;
    auto ret = getpwuid_r(uid, &pwd, buffer.data(), buffer.size(), &result);
    if (ret != 0 || result == nullptr) {
        SPDLOG_ERROR("Failed to get username for UID: {}", uid);
        return {};
    }
    return pwd.pw_name;
}

void ProcReader::GetProcList(novaagent::node::v1::ProcessInfoRequest* request) {
    if (total_memory_ == 0) {
        novaagent::node::v1::VirtualMemoryInfo memory;
        GetMemoryInfo(&memory);
        total_memory_ = memory.total();
        SPDLOG_INFO("machine memory info: {}", memory.ShortDebugString());
    }
    std::unordered_map<int, ProcessSampleInfo> process_table;
    novaagent::node::v1::ProcessInfoRequest curr;
    std::unordered_map<int, GPUProcessUsage> pid_gpu_usage;
    gpu_reader_.GetGPUProcessMemoryUsage(pid_gpu_usage);
    for (const auto& entry : director_iterator("/proc")) {
        if (!entry.is_directory()) {
            continue;
        }
        auto entry_name = entry.path().filename().string();
        pid_t pid = 0;
        if (!absl::SimpleAtoi(entry_name, &pid)) {
            continue;
        }
        ProcessSampleInfo sample;
        sample.process = curr.add_processes();
        sample.process->set_pid(pid);
        ProcessSampleInfo* prev = nullptr;
        if (auto it = process_table_.find(pid); it != process_table_.end()) {
            prev = &it->second;
        }

        GetProcDetail(entry.path().string(), sample, prev);

        if (auto it = pid_gpu_usage.find(pid); it != pid_gpu_usage.end()) {
            auto gpu_usage = sample.process->mutable_gpu_usage();
            gpu_usage->set_decoder_usage(it->second.dec_util);
            gpu_usage->set_encoder_usage(it->second.enc_util);
            gpu_usage->set_sm_usage(it->second.sm_util);
            gpu_usage->set_memory_used(it->second.memory);
        }
        process_table.insert({pid, sample});
    }
    request->CopyFrom(curr);
    {
        std::lock_guard<std::mutex> const guard(mutex_);
        process_table_.swap(process_table);
        last_.Swap(&curr);
    }
}

bool ProcReader::GetProcDetail(const std::string& path, ProcessSampleInfo& sample_info, ProcessSampleInfo* prev) {
    std::vector<std::tuple<std::string_view, bool,
                           std::function<bool(std::string_view, ::novaagent::node::v1::ProcessInfo*)>>> const
        parse_list = {
            {"status", false, [](auto content, auto process) { return ParseProcessStatus(content, process); }},
            {"statm", false,
             [this](auto content, auto process) {
                 return ParseProcessStatm(content, process->mutable_memory_usage());
             }},
            {"io", false, [&](auto content, auto) { return ParseProcessIO(content, sample_info, prev); }},
            {"stat", false, [&](auto content, auto) { return ParseProcessStat(content, sample_info, prev); }},
            {"cgroup", false,
             [](auto content, auto process) {
                 process->set_container_id(ParseProcessCgroup(content));
                 return true;
             }},
            {"cmdline", false,
             [](auto content, auto process) {
                 auto cmdline = process->mutable_cmdline();
                 for (auto& c : content) {
                     if (c == '\0') {
                         cmdline->push_back(' ');
                     } else {
                         cmdline->push_back(c);
                     }
                 }

                 while (!cmdline->empty() && cmdline->back() == ' ') {
                     cmdline->pop_back();
                 }
                 return true;
             }},
            {"exe", true,
             [](auto content, auto process) {
                 process->set_path(content);
                 return true;
             }},
            {"cwd", true,
             [](auto content, auto process) {
                 process->set_work_dir(content);
                 return true;
             }},
        };

    for (const auto& [filename, is_link, post_processor] : parse_list) {
        auto file_path = path + "/";
        file_path.append(filename);
        std::string content;
        bool ret;
        if (is_link) {
            ret = reader_->ReadFileLink(file_path, &content);
        } else {
            ret = reader_->ReadFile(file_path, &content);
        }
        if (ret) {
            post_processor(content, sample_info.process);
        } else {
            SPDLOG_ERROR("read file failed: {}", file_path);
        }
    }
    return true;
}

bool ProcReader::ParseProcessStatus(std::string_view content, ::novaagent::node::v1::ProcessInfo* process_info) {
    if (process_info == nullptr) {
        return false;
    }
    ProcessStatus result;
    if (!ParseProcessStatus(content, result)) {
        SPDLOG_ERROR("ParseProcessStatus failed.");
        return false;
    }
    process_info->set_name(result.name);
    process_info->set_ppid(result.ppid);
    process_info->set_uid(result.uid);
    process_info->set_gid(result.gid);
    process_info->set_state(GetProcessState(result.state));
    process_info->set_open_fd_count(result.fd_size);
    process_info->set_voluntary_ctx_switches(result.voluntary_ctxt_switches);
    process_info->set_involuntary_ctx_switches(result.nonvoluntary_ctxt_switches);
    process_info->set_threads_num(result.num_threads);
    process_info->set_user_name(GetUsernameFromUID(result.uid));
    process_info->set_is_kernel_thread(result.kthread == 1);
    return true;
}

bool ProcReader::ParseProcessStatus(std::string_view content, ProcessStatus& result) {
    std::vector<std::tuple<std::string_view, std::variant<std::string*, int64_t*, int*>, bool>> const parse_list = {
        {"Name:", &result.name, true},
        {"PPid:", &result.ppid, true},
        {"Uid:", &result.uid, true},
        {"Gid:", &result.gid, true},
        {"State:", &result.state, true},
        {"FDSize:", &result.fd_size, true},
        {"nonvoluntary_ctxt_switches:", &result.nonvoluntary_ctxt_switches, true},
        {"voluntary_ctxt_switches:", &result.voluntary_ctxt_switches, true},
        {"Threads:", &result.num_threads, true},
        {"VmSwap:", &result.swap, false},
        {"Kthread:", &result.kthread, false},
    };

    for (auto [key, field, required] : parse_list) {
        if (!ParseKeyVal(content, key, field, false) && required) {
            SPDLOG_ERROR("parser status file failed at key: {}", key);
            return false;
        }
    }

    result.username = GetUsernameFromUID(result.uid);
    return true;
}

bool ProcReader::ParseProcessStatm(std::string_view content, ::novaagent::node::v1::ProcessMemoryUsage* memory_info) {
    if (memory_info == nullptr) {
        return false;
    }
    constexpr size_t kNumFields = 7;
    std::vector<absl::string_view> fields = absl::StrSplit(content, ' ');
    if (fields.size() != kNumFields) {
        SPDLOG_ERROR("content filed not match, want={}, get={}", kNumFields, fields.size());
        return false;
    }

    std::array<long, kNumFields> buff = {0};
    for (size_t i = 0; i < kNumFields; ++i) {
        if (!absl::SimpleAtoi(fields[i], &buff[i])) {
            SPDLOG_ERROR("parse filed failed:{}", fields[i]);
            return false;
        }
        buff[i] *= kPageSize;
    }

    // The value is in pages, convert it to bytes
    memory_info->set_vm_size(buff[0]);
    memory_info->set_resident(buff[1]);
    memory_info->set_shared(buff[2]);
    memory_info->set_text(buff[3]);
    memory_info->set_lib(buff[4]);
    memory_info->set_data(buff[5]);
    memory_info->set_dirty(buff[6]);
    if (total_memory_ > 0) {
        double pct = buff[0];
        memory_info->set_vm_pct(pct / total_memory_);
        const double rss_pct = buff[1];
        memory_info->set_rss_pct(rss_pct / total_memory_);
    }
    return true;
}

bool ProcReader::ParseProcessStat(std::string_view content, ProcessSampleInfo& sample_info, ProcessSampleInfo* prev) {
    auto process_info = sample_info.process;
    if (process_info == nullptr) {
        return false;
    }

    ProcessStat result;
    if (!ParseProcessStat(content, result)) {
        SPDLOG_ERROR("ParseProcessStat failed");
        return false;
    }
    double uptime = NAN;
    if (!GetUptime(uptime)) {
        SPDLOG_ERROR("GetUptime failed");
        return false;
    }

    result.running_time = uptime - (result.running_time / kClockTicksPerSecond);
    result.utime /= kClockTicksPerSecond;
    result.stime /= kClockTicksPerSecond;

    process_info->set_start_time(time(nullptr) - result.running_time);
    process_info->set_running_time(result.running_time);
    process_info->set_cpu_user_time(result.utime);
    process_info->set_cpu_system_time(result.stime);

    clock_gettime(CLOCK_MONOTONIC, &sample_info.cpu_sample_ts);
    if (prev != nullptr) {
        double const diff_utime = process_info->cpu_user_time() - prev->process->cpu_user_time();
        double const diff_system_time = process_info->cpu_system_time() - prev->process->cpu_system_time();
        double const total = diff_utime + diff_system_time;
        double const time_diff = sample_info.CPUDiffInSeconds(*prev);
        if (time_diff > 0) {
            process_info->set_cpu_user_pct(diff_utime / time_diff);
            process_info->set_cpu_system_pct(diff_system_time / time_diff);
            process_info->set_cpu_total_pct(total / time_diff);
        }
    } else {
        auto total = result.utime + result.stime;
        process_info->set_cpu_user_pct(result.utime / result.running_time);
        process_info->set_cpu_system_pct(result.stime / result.running_time);
        process_info->set_cpu_total_pct(total / result.running_time);
    }

    return true;
}

bool ProcReader::ParseProcessStat(std::string_view content, ProcessStat& result) {
    std::vector<absl::string_view> fields = absl::StrSplit(content, ' ');
    constexpr size_t kFieldSize = 52;
    if (fields.size() != kFieldSize) {
        SPDLOG_ERROR("Invalid number of fields in /proc/<pid>/stat file");
        return false;
    }
    constexpr int kUserTimeField = 13;
    constexpr int kSystemTimeField = 14;
    constexpr int kNiceField = 18;
    constexpr int kStartTimeField = 21;

    std::vector<std::pair<std::string_view, std::variant<int*, double*, uint64_t*>>> const parse_list{
        {fields[kStartTimeField], &result.running_time},
        {fields[kNiceField], &result.nice},
        {fields[kUserTimeField], &result.utime},
        {fields[kSystemTimeField], &result.stime},
    };

    for (auto [key, target] : parse_list) {
        if (!ParseStr2Number(key, target, true)) {
            SPDLOG_ERROR("parse field failed for: {}", key);
            return false;
        }
    }
    return true;
}

long ProcReader::GetBootTime() {
    std::string content;
    if (!reader_->ReadFile("/proc/stat", &content)) {
        SPDLOG_ERROR("Failed to read /proc/stat");
        return 0;
    }
    StatInfo stat;
    if (!ParseStat(content, stat)) {
        return 0;
    }
    return stat.boot_time;
}

bool ProcReader::GetProcStat(StatInfo& result) {
    std::string content;
    if (!reader_->ReadFile("/proc/stat", &content)) {
        return false;
    }
    return ParseStat(content, result);
}

bool ProcReader::ParseStat(std::string_view content, StatInfo& result) {
    if (!ReadKeyVal(content, "btime", &result.boot_time, false)) {
        SPDLOG_ERROR("get boot time failed");
        return false;
    }
    std::string buffer;
    if (!ReadKeyVal(content, "cpu", &buffer, true)) {
        return false;
    }
    std::vector<absl::string_view> const fields = absl::StrSplit(buffer, ' ');
    constexpr size_t kFieldSize = 10;
    if (fields.size() != kFieldSize) {
        return false;
    }
    std::vector<uint64_t*> target{
        &result.cpu_time.user,   &result.cpu_time.nice, &result.cpu_time.system,  &result.cpu_time.idle,
        &result.cpu_time.iowait, &result.cpu_time.irq,  &result.cpu_time.softirq, &result.cpu_time.steal,
    };
    for (size_t i = 0; i < target.size(); i++) {
        if (!absl::SimpleAtoi(fields[i], target[i])) {
            return false;
        }
    }
    return true;
}

bool ProcReader::ParseUptime(std::string_view content, UptimeInfo& result) {
    // break the content into fields with space as delimiter
    std::vector<absl::string_view> const fields = absl::StrSplit(content, ' ');
    constexpr size_t kFieldSize = 2;
    if (fields.size() != kFieldSize) {
        SPDLOG_ERROR("Invalid number of fields in /proc/uptime file");
        return false;
    }

    if (!absl::SimpleAtod(fields[0], &result.uptime)) {
        SPDLOG_ERROR("Failed to parse uptime");
        return false;
    }
    if (!absl::SimpleAtod(fields[1], &result.idle_time)) {
        SPDLOG_ERROR("Failed to parse idle time");
        return false;
    }
    return true;
}

bool ProcReader::GetUptime(double& uptime) {
    std::string content;
    if (!reader_->ReadFile("/proc/uptime", &content)) {
        SPDLOG_ERROR("Failed to read /proc/uptime");
        return false;
    }

    UptimeInfo result;
    if (!ParseUptime(content, result)) {
        return false;
    }
    uptime = result.uptime;
    return true;
}

std::string_view ProcReader::ParseProcessCgroup(std::string_view content) {
    // current now, only support v2
    // 0::/system.slice/docker-a1e311027d69c40fe0520f086f7b85bcc62f31ac97b47f1b03d68b494f432b50.scope
    constexpr std::string_view key = "0::/system.slice/docker-";
    auto start = content.find(key);
    if (start == std::string::npos) {
        return {};
    }
    const auto cgroup = content.substr(key.size());
    auto end = cgroup.find(".scope");
    if (end == std::string::npos) {
        return {};
    }
    return cgroup.substr(0, end);
}

bool ProcReader::ParseLoadavg(std::string_view content, LoadAvgInfo& result) {
    std::vector<absl::string_view> const fields = absl::StrSplit(content, ' ');
    constexpr size_t kFieldSize = 5;
    if (fields.size() != kFieldSize) {
        SPDLOG_ERROR("Invalid number of fields in /proc/loadavg file");
        return false;
    }
    std::vector<absl::string_view> const subfields = absl::StrSplit(fields[3], '/');
    if (subfields.size() != 2) {
        SPDLOG_ERROR("Invalid number of fields in /proc/loadavg file");
        return false;
    }

    const std::vector<std::pair<std::string_view, std::variant<int*, double*>>> parse_list = {
        {fields[0], &result.avg_1},      {fields[1], &result.avg_5},    {fields[2], &result.avg_15},
        {subfields[0], &result.running}, {subfields[1], &result.total},
    };

    for (const auto& [str, target] : parse_list) {
        if (!ParseStr2Number(str, target, false)) {
            SPDLOG_ERROR("Invalid number of fields in /proc/loadavg file, field: {}", str);
            return false;
        }
    }
    return true;
}

bool ProcReader::ParseProcessIO(std::string_view content, ProcessSampleInfo& sample_info, ProcessSampleInfo* prev) {
    ProcessIO result;
    if (!ParseProcessIO(content, result)) {
        return false;
    }
    auto process_info = sample_info.process;
    clock_gettime(CLOCK_MONOTONIC, &sample_info.io_sample_ts);
    process_info->set_iostat_read_bytes(result.read_bytes);
    process_info->set_iostat_write_bytes(result.write_bytes);
    if (prev != nullptr) {
        double const diff_read_bytes = result.read_bytes - prev->process->iostat_read_bytes();
        double const diff_write_bytes = result.write_bytes - prev->process->iostat_write_bytes();
        double const diff_time = sample_info.IODiffInSeconds(*prev);
        if (diff_time > 0) {
            process_info->set_iostat_read_bytes_rate(diff_read_bytes / diff_time);
            process_info->set_iostat_write_bytes_rate(diff_write_bytes / diff_time);
        }
    }
    return true;
}

bool ProcReader::ParseProcessIO(std::string_view content, ProcessIO& result) {
    std::vector<std::pair<std::string_view, uint64_t*>> const parse_list = {
        {"read_bytes:", &result.read_bytes},
        {"write_bytes:", &result.write_bytes},
    };
    for (auto [key, target] : parse_list) {
        if (!ReadKeyVal(content, key, target, true)) {
            SPDLOG_ERROR("parse key{}: {}");
            return false;
        }
    }
    return true;
}

void ProcReader::GetMemoryInfo(novaagent::node::v1::VirtualMemoryInfo* memory_info) {
    if (memory_info == nullptr) {
        return;
    }
    std::string const cmdline_path = "/proc/meminfo";
    std::string content;
    if (!reader_->ReadFile(cmdline_path, &content)) {
        SPDLOG_ERROR("read file failed: {}", cmdline_path);
        return;
    }
    ProcMemoryInfo info;
    if (!ParseProcMemoryInfo(content, info)) {
        SPDLOG_ERROR("read file failed: {}", cmdline_path);
        return;
    }
    memory_info->set_free(info.free);
    memory_info->set_total(info.total);
    memory_info->set_swap_cached(info.cached);
    memory_info->set_used(info.used());
}

bool ProcReader::ParseProcMemoryInfo(std::string_view content, ProcMemoryInfo& memory_info) {
    std::vector<std::pair<std::string_view, uint64_t*>> const parse_list{
        {"MemTotal:", &memory_info.total},
        {"MemFree:", &memory_info.free},
        {"Cached:", &memory_info.cached},
        {"Buffers:", &memory_info.buffers},
    };

    for (auto [key, target] : parse_list) {
        if (!ReadKeyVal(content, key, target, false)) {
            SPDLOG_ERROR("parse field failed for: {}", key);
            return false;
        }
        *target *= 1024; // kB to B
    }
    return true;
}

std::unordered_map<int, ProcessSampleInfo> ProcReader::GetProcessTable() const {
    std::lock_guard<std::mutex> const guard(mutex_);
    return process_table_;
}

bool ProcReader::GetNetDev(NetDevList& result) {
    std::string content;
    if (!reader_->ReadFile("/proc/net/dev", &content)) {
        return false;
    }
    return ParseNetDev(content, result);
}

bool ProcReader::ParseNetDev(std::string_view content, NetDevList& result) {
    // split with newline
    std::vector<std::string_view> lines = absl::StrSplit(content, '\n');

    for (auto& line : lines) {
        std::vector<std::string_view> fields = absl::StrSplit(line, ' ');
        if (fields.size() != 17) {
            continue;
        }
        NetDevInfo info;
        info.dev_name = fields[0];
        if (!info.dev_name.empty()) {
            info.dev_name.pop_back();
        }
        absl::SimpleAtoi(fields[1], &info.rx_bytes);
        absl::SimpleAtoi(fields[2], &info.rx_packets);
        absl::SimpleAtoi(fields[9], &info.tx_bytes);
        absl::SimpleAtoi(fields[10], &info.tx_packets);
        result.dev_list.push_back(info);
    }
    return true;
}

} // namespace App::Source::Host::Collector::Process
