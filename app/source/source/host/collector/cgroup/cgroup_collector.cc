#include "source/host/collector/cgroup/cgroup_collector.h"
#include "common/parser.h"
#include "node/v1/info.pb.h"
#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <unordered_set>

using novaagent::node::v1::CGroupCPUMax;
using novaagent::node::v1::CGroupCPUStat;
using novaagent::node::v1::CGroupInfo;
using novaagent::node::v1::CGroupIOMax;
using novaagent::node::v1::CGroupMemoryEvent;
using novaagent::node::v1::CGroupMemoryStat;
using novaagent::node::v1::CGroupMemorySwapEvent;
using novaagent::node::v1::CGroupMount;
using novaagent::node::v1::CGroupNumaStat;
using novaagent::node::v1::CGroupStat;
using novaagent::node::v1::Pressure;
using novaagent::node::v1::PSI;

namespace App::Source::Host::Collector::CGroup {
std::string GetCgroupVersion() {
    const char* filename = "/sys/fs/cgroup/cgroup.controllers";
    if (access(filename, F_OK) == 0) {
        return "v2";
    } else {
        return "v1";
    }
}

template <typename T> T GetValue(std::string_view content, std::string_view key, bool required = true) {
    T ret{};
    if (!App::Common::ReadKeyVal(content, key, &ret, true, required)) {
        SPDLOG_ERROR("parse failed, content={}, key={}", content, key);
        return {};
    }
    return ret;
}

template <typename T> T Str2Number(const std::string_view& content) {
    T ret{};
    if constexpr (std::is_integral_v<T>) {
        if (!absl::SimpleAtoi(content, &ret)) {
            SPDLOG_ERROR("parse failed, content={}", content);
            return {};
        }
    } else if constexpr (std::is_same_v<T, double>) {
        if (!absl::SimpleAtod(content, &ret)) {
            SPDLOG_ERROR("parse failed, content={}", content);
            return {};
        }
    } else if constexpr (std::is_same_v<T, float>) {
        if (!absl::SimpleAtof(content, &ret)) {
            SPDLOG_ERROR("parse failed, content={}", content);
            return {};
        }
    } else {
        static_assert(false, "unsupported type");
    }
    return ret;
}

void ParseCGroupCPUMax(const std::string& content, CGroupCPUMax* max) {
    std::vector<std::string_view> fields = absl::StrSplit(content, ' ');
    if (fields.size() != 2) {
        return;
    }
    auto value = fields[0];
    if (value == "max") {
        max->set_max_string("max");
    } else {
        max->set_max_value(Str2Number<int64_t>(value));
    }
    max->set_period(Str2Number<int64_t>(fields[1]));
}

void ParseCPUSetList(const std::string& content, ::google::protobuf::RepeatedField<::int32_t>* list) {
    auto fields = absl::StrSplit(content, ',');
    for (const auto& field : fields) {
        std::vector<std::string_view> range = absl::StrSplit(field, '-');
        if (range.size() == 1) {
            list->Add(Str2Number<int64_t>(range[0]));
        } else {
            auto start = Str2Number<int64_t>(range[0]);
            auto end = Str2Number<int64_t>(range[1]);
            for (int i = start; i <= end; i++) {
                list->Add(i);
            }
        }
    }
}

void ParseCGroupStat(const std::string& content, CGroupStat* stat) {
    stat->set_nr_descendants(GetValue<int32_t>(content, "nr_descendants"));
    stat->set_nr_dying_descendants(GetValue<int32_t>(content, "nr_dying_descendants"));
}

// sample
// some avg10=0.00 avg60=0.00 avg300=0.00 total=582368
// full avg10=0.00 avg60=0.00 avg300=0.00 total=543832
// todo: clean code
void ParseCGroupPressure(const std::string& content, Pressure* pressure) {
    auto lines = absl::StrSplit(content, '\n');
    for (const auto& line : lines) {
        std::vector<std::string_view> fields = absl::StrSplit(line, ' ');
        if (fields[0] == "some") {
            pressure->mutable_some()->set_avg10(GetValue<float>(fields[1], "avg10="));
            pressure->mutable_some()->set_avg60(GetValue<float>(fields[2], "avg60="));
            pressure->mutable_some()->set_avg300(GetValue<float>(fields[3], "avg300="));
            pressure->mutable_some()->set_total(GetValue<int32_t>(fields[4], "total="));
        } else if (fields[0] == "full") {
            pressure->mutable_full()->set_avg10(GetValue<float>(fields[1], "avg10="));
            pressure->mutable_full()->set_avg60(GetValue<float>(fields[2], "avg60="));
            pressure->mutable_full()->set_avg300(GetValue<float>(fields[3], "avg300="));
            pressure->mutable_full()->set_total(GetValue<float>(fields[4], "total="));
        }
    }
}

void ParseCGroupCPUStat(const std::string& content, CGroupCPUStat* stat) {
    stat->set_usage_usec(GetValue<int64_t>(content, "usage_usec", false));
    stat->set_user_usec(GetValue<int64_t>(content, "user_usec", false));
    stat->set_system_usec(GetValue<int64_t>(content, "system_usec", false));
    stat->set_core_sched_force_idle_usec(GetValue<int64_t>(content, "core_sched.force_idle_usec", false));
    stat->set_nr_periods(GetValue<int64_t>(content, "nr_periods", false));
    stat->set_nr_throttled(GetValue<int64_t>(content, "nr_throttled", false));
    stat->set_throttled_usec(GetValue<int64_t>(content, "throttled_usec", false));
    stat->set_nr_bursts(GetValue<int64_t>(content, "nr_bursts", false));
    stat->set_burst_usec(GetValue<int64_t>(content, "burst_usec", false));
}

void ParseMemoryEvents(const std::string& content, CGroupMemoryEvent* events) {
    events->set_low(GetValue<int64_t>(content, "low"));
    events->set_high(GetValue<int64_t>(content, "high"));
    events->set_max(GetValue<int64_t>(content, "max"));
    events->set_oom(GetValue<int64_t>(content, "oom"));
}

void ParseMemorySwapEvents(const std::string& content, CGroupMemorySwapEvent* events) {
    events->set_high(GetValue<int64_t>(content, "high"));
    events->set_max(GetValue<int64_t>(content, "max"));
    events->set_fail(GetValue<int64_t>(content, "fail"));
}

void ParseNumaStat(const std::string& content, CGroupNumaStat* stat) {
}

void ParseMemoryStat(const std::string& content, CGroupMemoryStat* stat) {
}

// 8:16 rbps=2097152 wbps=max riops=max wiops=max
void ParseIOMax(const std::string& content,
                ::google::protobuf::RepeatedPtrField<::novaagent::node::v1::CGroupIOMax>* dst) {
    if (content.empty()) {
        return;
    }
    auto lines = absl::StrSplit(content, '\n');
    for (auto& line : lines) {
        auto max = dst->Add();
        std::vector<std::string_view> fields = absl::StrSplit(line, ' ');
        auto rbps = GetValue<std::string>(fields[1], "rbps");
        if (rbps == "max") {
            max->set_rbps_string("max");
        } else {
            max->set_rbps_value(Str2Number<int32_t>(rbps));
        }
        auto wbps = GetValue<std::string>(fields[2], "wbps");
        if (wbps == "max") {
            max->set_rbps_string(wbps);
        } else {
            max->set_wbps_value(Str2Number<int32_t>(wbps));
        }
        auto riops = GetValue<std::string>(fields[3], "riops");
        if (riops == "max") {
            max->set_riops_string(riops);
        } else {
            max->set_riops_value(Str2Number<int32_t>(riops));
        }
        auto wiops = GetValue<std::string>(fields[4], "wiops");
        if (wiops == "max") {
            max->set_wiops_string("max");
        } else {
            max->set_wiops_value(Str2Number<int32_t>(wiops));
        }
    }
}

void CGroupCollector::handleFile(const std::filesystem::directory_entry& entry, CGroupMount* mount) {
    // ignore write-only files
    std::unordered_set<std::string_view> ignored = {"cgroup.subtree_control", "cgroup.kill", "memory.reclaim"};

    std::map<std::string, std::function<void(const std::string& content)>> table = {
        {"cgroup.controllers",
         [&](const std::string& content) {
             auto* cgroup = mount->mutable_cgroup();
             auto fields = absl::StrSplit(content, ' ');
             for (const auto& field : fields) {
                 cgroup->add_controllers(field);
             }
         }},
        {"cgroup.events",
         [&](const std::string& content) {
             auto* event = mount->mutable_cgroup()->mutable_events();
             event->set_populated(GetValue<int32_t>(content, "populated"));
             event->set_frozen(GetValue<int32_t>(content, "frozen"));
         }},
        {"cgroup.freeze",
         [&](const std::string& content) { mount->mutable_cgroup()->set_freeze(Str2Number<int>(content)); }},
        {"cgroup.max.depth",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_cgroup()->set_max_depth_string("max");
             } else {
                 mount->mutable_cgroup()->set_max_depth_value(Str2Number<int32_t>(content));
             }
         }},
        {"cgroup.max.descendants",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_cgroup()->set_max_descendants_string("max");
             } else {
                 mount->mutable_cgroup()->set_max_descendants_value(Str2Number<int32_t>(content));
             }
         }},
        {"cgroup.pressure",
         [&](const std::string& content) { mount->mutable_cgroup()->set_pressure(Str2Number<int32_t>(content)); }},
        {"cgroup.procs",
         [&](const std::string& content) {
             auto* cgroup = mount->mutable_cgroup();
             auto fields = absl::StrSplit(content, '\n');
             for (const auto& field : fields) {
                 cgroup->add_procs(Str2Number<int32_t>(field));
             }
         }},
        {"cgroup.stat",
         [&](const std::string& content) { ParseCGroupStat(content, mount->mutable_cgroup()->mutable_stat()); }},
        {"cgroup.threads",
         [&](const std::string& content) {
             auto* cgroup = mount->mutable_cgroup();
             auto fields = absl::StrSplit(content, '\n');
             for (const auto& field : fields) {
                 cgroup->add_threads(Str2Number<int32_t>(field));
             }
         }},
        {"cgroup.type", [&](const std::string& content) { mount->mutable_cgroup()->set_type(content); }},
        {"cpu.idle", [&](const std::string& content) { mount->mutable_cpu()->set_idle(Str2Number<int32_t>(content)); }},
        {"cpu.max",
         [&](const std::string& content) { ParseCGroupCPUMax(content, mount->mutable_cpu()->mutable_max()); }},
        {"cpu.max.burst",
         [&](const std::string& content) { mount->mutable_cpu()->set_max_burst(Str2Number<int32_t>(content)); }},
        {"cpu.pressure",
         [&](const std::string& content) { ParseCGroupPressure(content, mount->mutable_cpu()->mutable_pressure()); }},
        {"cpu.stat",
         [&](const std::string& content) { ParseCGroupCPUStat(content, mount->mutable_cpu()->mutable_stat()); }},
        {"cpu.stat.local",
         [&](const std::string& content) { ParseCGroupCPUStat(content, mount->mutable_cpu()->mutable_stat_local()); }},
        {"cpu.uclamp.max",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_cpu()->set_uclamp_max_string("max");
             } else {
                 mount->mutable_cpu()->set_uclamp_max_value(Str2Number<int32_t>(content));
             }
         }},
        {"cpu.uclamp.min",
         [&](const std::string& content) { mount->mutable_cpu()->set_uclamp_min(Str2Number<float>(content)); }},
        {"cpu.weight",
         [&](const std::string& content) { mount->mutable_cpu()->set_weight(Str2Number<int32_t>(content)); }},
        {"cpu.weight.nice",
         [&](const std::string& content) { mount->mutable_cpu()->set_weight_nice(Str2Number<int32_t>(content)); }},
        {"cpuset.cpus",
         [&](const std::string& content) { ParseCPUSetList(content, mount->mutable_cpuset()->mutable_cpus()); }},
        {"cpuset.cpus.effective",
         [&](const std::string& content) {
             ParseCPUSetList(content, mount->mutable_cpuset()->mutable_cpus_effective());
         }},
        {"cpuset.cpus.exclusive",
         [&](const std::string& content) {
             ParseCPUSetList(content, mount->mutable_cpuset()->mutable_cpus_exclusive());
         }},
        {"cpuset.cpus.exclusive.effective",
         [&](const std::string& content) {
             ParseCPUSetList(content, mount->mutable_cpuset()->mutable_cpus_exclusive_effective());
         }},
        {"cpuset.cpus.partition",
         [&](const std::string& content) { mount->mutable_cpuset()->set_cpus_partition(content); }},
        {"cpuset.mems",
         [&](const std::string& content) { ParseCPUSetList(content, mount->mutable_cpuset()->mutable_mems()); }},
        {"cpuset.mems.effective",
         [&](const std::string& content) {
             ParseCPUSetList(content, mount->mutable_cpuset()->mutable_mems_effective());
         }},
        {"io.max", [&](const std::string& content) { ParseIOMax(content, mount->mutable_io()->mutable_max()); }},
        {"io.pressure",
         [&](const std::string& content) { ParseCGroupPressure(content, mount->mutable_io()->mutable_pressure()); }},
        {"io.prio.class", [&](const std::string& content) {}},
        {"io.stat", [&](const std::string& content) {}},
        {"io.weight", [&](const std::string& content) {}},
        {"memory.current",
         [&](const std::string& content) { mount->mutable_memory()->set_current(Str2Number<int64_t>(content)); }},
        {"memory.events",
         [&](const std::string& content) { ParseMemoryEvents(content, mount->mutable_memory()->mutable_events()); }},
        {"memory.events.local",
         [&](const std::string& content) { ParseMemoryEvents(content, mount->mutable_memory()->mutable_events()); }},
        {"memory.high",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_memory()->set_high_string("max");
             } else {
                 mount->mutable_memory()->set_high_value(Str2Number<int64_t>(content));
             }
         }},
        {"memory.low",
         [&](const std::string& content) { mount->mutable_memory()->set_low(Str2Number<int64_t>(content)); }},
        {"memory.max",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_memory()->set_max_string("max");
             } else {
                 mount->mutable_memory()->set_max_value(Str2Number<int64_t>(content));
             }
         }},
        {"memory.min",
         [&](const std::string& content) { mount->mutable_memory()->set_min(Str2Number<int64_t>(content)); }},
        {"memory.numa_stat",
         [&](const std::string& content) { ParseNumaStat(content, mount->mutable_memory()->mutable_numa_stat()); }},
        {"memory.oom.group",
         [&](const std::string& content) { mount->mutable_memory()->set_oom_group(Str2Number<int32_t>(content)); }},
        {"memory.peak",
         [&](const std::string& content) { mount->mutable_memory()->set_peak(Str2Number<int64_t>(content)); }},
        {"memory.pressure",
         [&](const std::string& content) {
             ParseCGroupPressure(content, mount->mutable_memory()->mutable_pressure());
         }},
        {"memory.stat",
         [&](const std::string& content) { ParseMemoryStat(content, mount->mutable_memory()->mutable_stat()); }},
        {"memory.swap.current",
         [&](const std::string& content) { mount->mutable_memory()->set_swap_current(Str2Number<int64_t>(content)); }},
        {"memory.swap.events",
         [&](const std::string& content) {
             ParseMemorySwapEvents(content, mount->mutable_memory()->mutable_swap_events());
         }},
        {"memory.swap.high",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_memory()->set_swap_high_string("max");
             } else {
                 mount->mutable_memory()->set_swap_high_value(Str2Number<int64_t>(content));
             }
         }},
        {"memory.swap.max",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_memory()->set_swap_max_string("max");
             } else {
                 mount->mutable_memory()->set_swap_max_value(Str2Number<int64_t>(content));
             }
         }},
        {"memory.swap.peak",
         [&](const std::string& content) { mount->mutable_memory()->set_swap_peak(Str2Number<int64_t>(content)); }},
        {"memory.zswap.current",
         [&](const std::string& content) { mount->mutable_memory()->set_zswap_current(Str2Number<int64_t>(content)); }},
        {"memory.zswap.max",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_memory()->set_zswap_max_string("max");
             } else {
                 mount->mutable_memory()->set_zswap_max_value(Str2Number<int64_t>(content));
             }
         }},
        {"memory.zswap.writeback",
         [&](const std::string& content) {
             mount->mutable_memory()->set_zswap_writeback(Str2Number<int32_t>(content));
         }},
        {"pids.current",
         [&](const std::string& content) { mount->mutable_pids()->set_current(Str2Number<int32_t>(content)); }},
        {"pids.events",
         [&](const std::string& content) { mount->mutable_pids()->set_events(GetValue<int32_t>(content, "max")); }},
        {"pids.max",
         [&](const std::string& content) {
             if (content == "max") {
                 mount->mutable_pids()->set_max_string("max");
             } else {
                 mount->mutable_pids()->set_max_value(Str2Number<int32_t>(content));
             }
         }},
        {"pids.peak",
         [&](const std::string& content) { mount->mutable_pids()->set_peak(Str2Number<int32_t>(content)); }},
        {"misc.current", [&](const std::string& content) {}},
        {"misc.events", [&](const std::string& content) {}},
        {"rdma.current", [&](const std::string& content) {}},
        {"rdma.max", [&](const std::string& content) {}},
    };
    auto filename = entry.path().filename().string();
    if (ignored.find(filename) != ignored.end()) {
        return;
    }
    std::string content;
    if (!default_reader_.ReadFile(entry.path().string(), &content)) {
        SPDLOG_ERROR("unable to read file: {}", entry.path().string());
        return;
    }
    while (!content.empty() && content.back() == '\n') {
        content.pop_back();
    }
    if (content.empty()) {
        return;
    }
    if (table.find(filename) != table.end()) {
        table[filename](content);
    } else if (filename.size() > 8 && filename.substr(0, 8) == "hugetlb.") {
        handleHugetlb(filename, content, mount->mutable_hugetlb());
    } else {
        SPDLOG_WARN("unhandled file: {}", filename);
    }
}

void CGroupCollector::GetCGroupInfo(novaagent::node::v1::CGroupInfo& cgroup_info) {
    cgroup_info.set_version(GetCgroupVersion());
    std::unordered_map<std::string, CGroupMount*> table;
    auto* root = cgroup_info.mutable_cgroup_mount();
    root->set_mount_point("/sys/fs/cgroup");
    table["/sys/fs/cgroup"] = root;
    for (const auto& entry : std::filesystem::recursive_directory_iterator("/sys/fs/cgroup")) {
        if (entry.is_directory()) {
            auto parent = entry.path().parent_path().string();
            assert(table.find(parent) != table.end());
            auto curr = table[parent]->add_children();
            curr->set_mount_point(entry.path().string());
            table[entry.path().string()] = curr;
        } else if (entry.is_regular_file()) {
            auto iter = table.find(entry.path().parent_path().string());
            assert(iter != table.end());
            handleFile(entry, iter->second);
        }
    }
}

void CGroupCollector::handleHugetlb(const std::string& filename, const std::string& content,
                                    novaagent::node::v1::CGroupHugeTLBController* hugetlb) {
    // filename like this:
    // hugetlb.1GB.events
    // hugetlb.2MB.events
    std::vector<std::string_view> fields = absl::StrSplit(filename, '.');
    if (fields.size() < 3) {
        SPDLOG_WARN("unrecognized file: {}", filename);
        return;
    }
    std::string subname(fields[2]);
    for (size_t i = 3; i < fields.size(); i++) {
        subname.push_back('.');
        subname.append(fields[i]);
    }
    auto& tlb_entry = (*hugetlb->mutable_tlb())[fields[1]];
    auto parse_event = [&](const std::string& content, auto* b) {
        std::vector<std::string_view> fields = absl::StrSplit(content, ' ');
        if (fields.size() != 2) {
            return;
        }
        if (fields[0] == "max") {
            b->set_max_string("max");
        } else {
            b->set_max_value(Str2Number<int32_t>(fields[0]));
        }
        b->set_period(Str2Number<int32_t>(fields[1]));
    };

    std::unordered_map<std::string, std::function<void(const std::string& content)>> const table = {
        {"current", [&](const std::string& content) { tlb_entry.set_current(Str2Number<int32_t>(content)); }},
        {"events.local", [&](const std::string& content) { parse_event(content, tlb_entry.mutable_events_local()); }},
        {"events", [&](const std::string& content) { parse_event(content, tlb_entry.mutable_events()); }},
        {"max",
         [&](const std::string& content) {
             if (content == "max") {
                 tlb_entry.set_max_string("max");
             } else {
                 tlb_entry.set_max_value(Str2Number<int32_t>(content));
             }
         }},
        {"numa_stat", [&](const std::string& content) {}},
        {"rsvd.current", [&](const std::string& content) { tlb_entry.set_max_value(Str2Number<int32_t>(content)); }},
        {"rsvd.max",
         [&](const std::string& content) {
             if (content == "max") {
                 tlb_entry.set_rsvd_max_string("max");
             } else {
                 tlb_entry.set_rsvd_max_value(Str2Number<int32_t>(content));
             }
         }},
    };
    auto it = table.find(subname);
    if (it != table.end()) {
        it->second(content);
    } else {
        SPDLOG_WARN("unrecognized file: {}", filename);
    }
}

} // namespace App::Source::Host::Collector::CGroup
