#include "common/file.h"
#include "node/v1/info.pb.h"
#include "source/host/collector/process/proc_reader.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <sys/sysinfo.h>
#include <unistd.h>

using App::Common::FileReader;
using App::Source::Host::Collector::Process::LoadAvgInfo;
using App::Source::Host::Collector::Process::ProcessSampleInfo;
using App::Source::Host::Collector::Process::ProcessStat;
using App::Source::Host::Collector::Process::ProcReader;
using App::Source::Host::Collector::Process::StatInfo;
using App::Source::Host::Collector::Process::UptimeInfo;
using ::novaagent::node::v1::ProcessInfo;
using ::novaagent::node::v1::ProcessInfoRequest;
using ::novaagent::node::v1::ProcessState;

struct MockFileReader : public FileReader {
    MOCK_METHOD(bool, ReadFile, (const std::string&, std::string*), (override));
    MOCK_METHOD(bool, ReadFileLink, (const std::string&, std::string*), (override));
};

constexpr std::string_view status_content = R"(
Name:   systemd
Umask:  0000
State:  S (sleeping)
Tgid:   1
Ngid:   0
Pid:    1
PPid:   0
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 512
Groups:
NStgid: 1
NSpid:  1
NSpgid: 1
NSsid:  1
Kthread:        0
VmPeak:    23692 kB
VmSize:    23656 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     14188 kB
VmRSS:     14188 kB
RssAnon:            4736 kB
RssFile:            9452 kB
RssShmem:              0 kB
VmData:     4632 kB
VmStk:       132 kB
VmExe:        44 kB
VmLib:     12188 kB
VmPTE:        88 kB
VmSwap:        0 kB
HugetlbPages:          0 kB
CoreDumping:    0
THP_enabled:    1
untag_mask:     0xffffffffffffffff
Threads:        1
SigQ:   1/63506
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 7fefc1fe28014a03
SigIgn: 0000000000001000
SigCgt: 00000000000004ec
CapInh: 0000000000000000
CapPrm: 000001ffffffffff
CapEff: 000001ffffffffff
CapBnd: 000001ffffffffff
CapAmb: 0000000000000000
NoNewPrivs:     0
Seccomp:        0
Seccomp_filters:        0
Speculation_Store_Bypass:       thread vulnerable
SpeculationIndirectBranch:      conditional enabled
Cpus_allowed:   ffffffff,ffffffff,ffffffff,ffffffff
Cpus_allowed_list:      0-127
Mems_allowed:   00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000001
Mems_allowed_list:      0
voluntary_ctxt_switches:        4408
nonvoluntary_ctxt_switches:     1178
x86_Thread_features:
x86_Thread_features_locked:
)";

constexpr std::string_view proc_stat = R"(
cpu  236027 46 470727 13045539 3931 0 23618 0 0 0
cpu0 31099 9 56247 1639252 427 0 7702 0 0 0
cpu1 26182 1 65483 1616400 562 0 5288 0 0 0
cpu2 28315 0 60098 1619767 464 0 2447 0 0 0
cpu3 26369 11 65323 1635799 465 0 1882 0 0 0
cpu4 30713 1 58009 1625293 618 0 1543 0 0 0
cpu5 39607 2 53677 1629458 541 0 1271 0 0 0
cpu6 23781 18 55463 1641358 454 0 1076 0 0 0
cpu7 29959 1 56423 1638209 397 0 2406 0 0 0
intr 50776094 42 23 0 0 0 0 2 0 0 0 0 0 11619 0 0 0 205938 196642 434935 976874 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 9218 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
ctxt 74164726
btime 1725761152
processes 168810
procs_running 2
procs_blocked 0
softirq 15430298 35995 1767039 20 1196621 253386 0 149600 8008479 0 4019158
)";

TEST(TestProcReader, ParseProcessStatus) {
    ProcReader const reader;
    EXPECT_FALSE(reader.ParseProcessStatus(status_content, nullptr));

    ::novaagent::node::v1::ProcessInfo process_info;
    EXPECT_TRUE(reader.ParseProcessStatus(status_content, &process_info));
    EXPECT_EQ(process_info.name(), "systemd");
    EXPECT_EQ(process_info.ppid(), 0);
    EXPECT_EQ(process_info.state(), ProcessState::Sleeping);
    EXPECT_EQ(process_info.uid(), 0);
    EXPECT_EQ(process_info.gid(), 0);
    EXPECT_EQ(process_info.user_name(), "root");
    EXPECT_EQ(process_info.open_fd_count(), 512);
    EXPECT_EQ(process_info.involuntary_ctx_switches(), 1178);
    EXPECT_EQ(process_info.voluntary_ctx_switches(), 4408);
    EXPECT_EQ(process_info.threads_num(), 1);
}

TEST(TestProcReader, ParseProcessStatm) {
    std::string content = "5914 3547 2363 11 0 1191 0";
    ProcReader reader;
    ::novaagent::node::v1::ProcessMemoryUsage memory;
    EXPECT_TRUE(reader.ParseProcessStatm(content, &memory));
    constexpr int kPageSize = 4096;
    EXPECT_EQ(memory.vm_size(), 5914 * kPageSize);
    EXPECT_EQ(memory.resident(), 3547 * kPageSize);
    EXPECT_EQ(memory.shared(), 2363 * kPageSize);
    EXPECT_EQ(memory.text(), 11 * kPageSize);
    EXPECT_EQ(memory.lib(), 0);
    EXPECT_EQ(memory.data(), 1191 * kPageSize);
    EXPECT_EQ(memory.dirty(), 0);
}

TEST(TestProcReader, ParseProcessStat) {
    std::string const stat_content =
        "1 (systemd) S 0 1 1 0 -1 4194560 47321 1185226 108 6935 300 800 1372 5131 20 0 1 0 143 24223744 3547 "
        "18446744073709551615 1 1 0 0 0 0 671173123 4096 1260 0 0 0 17 3 0 0 0 0 0 0 0 0 0 0 0 0 0";
    MockFileReader mock_reader;
    EXPECT_CALL(mock_reader, ReadFile("/proc/uptime", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = "342.51 2233.31";
            return true;
        });
    ProcReader reader(&mock_reader);
    ::novaagent::node::v1::ProcessInfo process;
    ProcessSampleInfo sample_info;
    sample_info.process = &process;
    EXPECT_TRUE(reader.ParseProcessStat(stat_content, sample_info, nullptr));
    EXPECT_NEAR(process.running_time(), 341, 0.1);
    EXPECT_EQ(process.cpu_user_time(), 3);
    EXPECT_EQ(process.cpu_system_time(), 8);
    EXPECT_NEAR(process.cpu_user_pct(), 0.008, 0.001);
    EXPECT_NEAR(process.cpu_system_pct(), 0.023, 0.001);
    EXPECT_NEAR(process.cpu_total_pct(), 0.032, 0.001);
    EXPECT_EQ(process.cpu_nice(), 0);
}

TEST(TestProcReader, ParseProcessStatExport) {
    std::string const stat_content =
        "1 (systemd) S 0 1 1 0 -1 4194560 47321 1185226 108 6935 300 800 1372 5131 20 0 1 0 143 24223744 3547 "
        "18446744073709551615 1 1 0 0 0 0 671173123 4096 1260 0 0 0 17 3 0 0 0 0 0 0 0 0 0 0 0 0 0";
    ProcReader reader;
    ProcessStat result;
    EXPECT_TRUE(reader.ParseProcessStat(stat_content, result));
    EXPECT_EQ(result.running_time, 143);
    EXPECT_EQ(result.stime, 800);
    EXPECT_EQ(result.utime, 300);
    EXPECT_EQ(result.nice, 0);
}

TEST(TestProcReader, ParseProcessCgroup) {
    std::string const stat_content =
        "0::/system.slice/docker-a1e311027d69c40fe0520f086f7b85bcc62f31ac97b47f1b03d68b494f432b50.scope";
    ProcReader reader;
    EXPECT_EQ(reader.ParseProcessCgroup(stat_content),
              "a1e311027d69c40fe0520f086f7b85bcc62f31ac97b47f1b03d68b494f432b50");
    EXPECT_EQ(reader.ParseProcessCgroup("0::/init.scope"), "");
    EXPECT_EQ(reader.ParseProcessCgroup(""), "");
}

TEST(TestProcReader, ParseUptime) {
    ProcReader reader;
    UptimeInfo stat;
    EXPECT_FALSE(reader.ParseUptime("", stat));
    EXPECT_FALSE(reader.ParseUptime("19491.47", stat));
    EXPECT_FALSE(reader.ParseUptime("19491.47 135829.30 1", stat));
    EXPECT_TRUE(reader.ParseUptime("19491.47 135829.30", stat));
    EXPECT_EQ(stat.uptime, 19491.47);
    EXPECT_EQ(stat.idle_time, 135829.30);
}

TEST(TestProcReader, ParseStat) {
    ProcReader reader;
    StatInfo stat;
    EXPECT_FALSE(reader.ParseStat("", stat));
    EXPECT_FALSE(reader.ParseStat("19491.47", stat));
    EXPECT_TRUE(reader.ParseStat(proc_stat, stat));
    EXPECT_EQ(stat.boot_time, 1725761152);
}

TEST(TestProcReader, ParseLoadavg) {
    ProcReader reader;
    LoadAvgInfo stat;
    EXPECT_FALSE(reader.ParseLoadavg("", stat));
    EXPECT_FALSE(reader.ParseLoadavg("19491.47", stat));

    EXPECT_TRUE(reader.ParseLoadavg("3.21 2.49 2.14 1/1040 25370", stat));
    EXPECT_EQ(stat.avg_1, 3.21);
    EXPECT_EQ(stat.avg_5, 2.49);
    EXPECT_EQ(stat.avg_15, 2.14);
    EXPECT_EQ(stat.running, 1);
    EXPECT_EQ(stat.total, 1040);
}

constexpr std::string_view mock_status = R"(
Name:   program
Umask:  0002
State:  S (sleeping)
Tgid:   178776
Ngid:   0
Pid:    178776
PPid:   41833
TracerPid:      0
Uid:    1000    1000    1000    1000
Gid:    1000    1000    1000    1000
FDSize: 256
Groups: 4 24 27 30 46 100 114 124 1000
NStgid: 178776
NSpid:  178776
NSpgid: 178776
NSsid:  41833
Kthread:        0
VmPeak:     8292 kB
VmSize:     8292 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:      1920 kB
VmRSS:      1920 kB
RssAnon:               0 kB
RssFile:            1920 kB
RssShmem:              0 kB
VmData:      224 kB
VmStk:       136 kB
VmExe:        16 kB
VmLib:      1748 kB
VmPTE:        56 kB
VmSwap:        0 kB
HugetlbPages:          0 kB
CoreDumping:    0
THP_enabled:    1
untag_mask:     0xffffffffffffffff
Threads:        1
SigQ:   0/63506
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000000000
SigCgt: 0000000000000000
CapInh: 0000000000000000
CapPrm: 0000000000000000
CapEff: 0000000000000000
CapBnd: 000001ffffffffff
CapAmb: 0000000000000000
NoNewPrivs:     0
Seccomp:        0
Seccomp_filters:        0
Speculation_Store_Bypass:       thread vulnerable
SpeculationIndirectBranch:      conditional enabled
Cpus_allowed:   ffffffff,ffffffff,ffffffff,ffffffff
Cpus_allowed_list:      0-127
Mems_allowed:   00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000001
Mems_allowed_list:      0
voluntary_ctxt_switches:        1
nonvoluntary_ctxt_switches:     0
x86_Thread_features:
x86_Thread_features_locked:
)";

TEST(TestProcReader, GetProcDetail) {
    MockFileReader mock_reader;
    ProcReader reader(&mock_reader);
    EXPECT_CALL(mock_reader, ReadFile("/proc/178776/status", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = mock_status;
            return true;
        });

    EXPECT_CALL(mock_reader, ReadFile("/proc/178776/statm", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = "2073 480 480 4 0 90 0";
            return true;
        });

    EXPECT_CALL(mock_reader, ReadFile("/proc/178776/io", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = R"(
rchar: 44175
wchar: 14968
syscr: 142
syscw: 130
read_bytes: 5
write_bytes: 6
cancelled_write_bytes: 0
)";
            return true;
        });

    EXPECT_CALL(mock_reader, ReadFile("/proc/178776/stat", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content =
                "178776 (sleep) S 41833 178776 41833 34817 178776 4194304 98 0 0 0 4368 6552 0 0 20 0 1 0 3018773 "
                "8491008 480 18446744073709551615 103806683475968 103806683489969 140728134632112 0 0 0 0 0 0 1 "
                "0 0 17 4 0 0 0 0 0 103806683499472 103806683500696 103806716895232 140728134639539 "
                "140728134639551 140728134639551 140728134643689 0";
            return true;
        });
    EXPECT_CALL(mock_reader, ReadFile("/proc/uptime", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = "32372.10 226830.14";
            return true;
        });
    EXPECT_CALL(mock_reader, ReadFile("/proc/178776/cgroup", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = "0::/user.slice/user-1000.slice/session-4.scope";
            return true;
        });
    EXPECT_CALL(mock_reader, ReadFile("/proc/178776/cmdline", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = "/path/to/program --version";
            content->at(16) = '\0';
            return true;
        });
    EXPECT_CALL(mock_reader, ReadFileLink("/proc/178776/exe", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = "/path/to/program";
            return true;
        });
    EXPECT_CALL(mock_reader, ReadFileLink("/proc/178776/cwd", testing::_))
        .WillOnce([](const std::string& /*path*/, std::string* content) {
            *content = "/work_dir";
            return true;
        });
    ProcessInfo process;
    ProcessSampleInfo sample_info;
    sample_info.process = &process;
    process.set_pid(178776);
    reader.GetProcDetail("/proc/178776", sample_info, nullptr);

    EXPECT_EQ(process.pid(), 178776);
    EXPECT_EQ(process.name(), "program");
    EXPECT_EQ(process.path(), "/path/to/program");
    EXPECT_EQ(process.cmdline(), "/path/to/program --version");
    EXPECT_EQ(process.ppid(), 41833);
    EXPECT_EQ(process.uid(), 1000);
    //    EXPECT_EQ(process.user_name(), "user"); // todo
    EXPECT_EQ(process.gid(), 1000);
    EXPECT_EQ(process.state(), ProcessState::Sleeping);
    EXPECT_FLOAT_EQ(process.running_time(), 2184.37);
    EXPECT_EQ(process.container_id(), "");
    EXPECT_EQ(process.open_fd_count(), 256);
    EXPECT_EQ(process.involuntary_ctx_switches(), 0);
    EXPECT_EQ(process.voluntary_ctx_switches(), 1);
    EXPECT_EQ(process.cpu_nice(), 0);
    EXPECT_EQ(process.threads_num(), 1);
    // todo
    //    EXPECT_EQ(process.env_variables(), );
    //    EXPECT_EQ(process.memory_usage(), );
    EXPECT_EQ(process.iostat_read_bytes(), 5);
    EXPECT_EQ(process.iostat_write_bytes(), 6);
    EXPECT_EQ(process.iostat_read_bytes_rate(), 0);
    EXPECT_EQ(process.iostat_write_bytes_rate(), 0);

    EXPECT_EQ(process.cpu_user_time(), 43);
    EXPECT_EQ(process.cpu_system_time(), 65);
    EXPECT_NEAR(process.cpu_user_pct(), 0.02, 0.01);
    EXPECT_NEAR(process.cpu_system_pct(), 0.03, 0.01);
    EXPECT_NEAR(process.cpu_total_pct(), 0.05, 0.01);

    EXPECT_EQ(process.work_dir(), "/work_dir");
}

TEST(TestProcReader, DISABLED_GetProcessList) {
    ProcReader reader;
    ProcessInfoRequest request;
    reader.GetProcList(&request);

    for (auto& proc : request.processes()) {
        SPDLOG_INFO("proc: {}", proc.DebugString());
    }
    SPDLOG_INFO("process size: {}", request.processes_size());
}

TEST(TestProcReader, Sample) {
    ProcReader reader;
    ProcessInfo process;
    ProcessSampleInfo sample_info;
    sample_info.process = &process;
    process.set_pid(1615);
    reader.GetProcDetail("/proc/1615", sample_info, nullptr);

    SPDLOG_INFO("process info: {}", process.ShortDebugString());
}

TEST(TestProcReader, GetMemoryInfo) {
    ProcReader reader;
    novaagent::node::v1::VirtualMemoryInfo memory;
    reader.GetMemoryInfo(&memory);
    SPDLOG_INFO("machine memory size: {}", memory.ShortDebugString());

    struct sysinfo info{};
    EXPECT_EQ(sysinfo(&info), 0);
    EXPECT_EQ(info.totalram, memory.total());
    //    EXPECT_EQ(info.freeram, memory.free());
    //    EXPECT_EQ(info.totalswap, memory.swap_cached());
}
