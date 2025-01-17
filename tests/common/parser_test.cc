#include "common/parser.h"
#include <gtest/gtest.h>
#include <utility>
#include <vector>
using App::Common::ConvertStr2Number;
using App::Common::ParseStr2Number;
using App::Common::ReadKeyVal;

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

TEST(TestParser, ConvertStr2Number) {
    auto check = [](std::string_view str, bool simple, auto target) {
        decltype(target) value;
        EXPECT_TRUE(ConvertStr2Number(str, &value, simple));
        EXPECT_EQ(value, target);
    };
    constexpr int kExpect = 123;
    check("123", true, kExpect);
    check("+123", true, kExpect);
    check("-123", true, -kExpect);
    check(" 123", true, kExpect);
    check(" 123 ", true, kExpect);
    check(" 123 ", true, kExpect);
    check("123 2", false, kExpect);

    // false example
    int value = 0;
    EXPECT_FALSE(ConvertStr2Number("123 456", &value, true));
    EXPECT_FALSE(ConvertStr2Number("       ", &value, true));
    EXPECT_FALSE(ConvertStr2Number("", &value, true));
}

TEST(TestParser, ParseStr2Number) {
    int i0, i1;
    double d0, d1;
    float f0, f1;
    const std::vector<std::pair<std::string_view, std::variant<int*, double*, float*>>> list = {
        {"123", &i0}, {"123", &f0}, {"123", &d0}, {"123 456", &i1}, {"123 456", &f1}, {"123 456", &d1},
    };

    for (auto [str, target] : list) {
        EXPECT_TRUE(ParseStr2Number(str, target, false)) << str;
    }
    EXPECT_EQ(i0, 123);
    EXPECT_EQ(i1, 123);
    EXPECT_EQ(f0, 123);
    EXPECT_EQ(f1, 123);
    EXPECT_EQ(d0, 123);
    EXPECT_EQ(d1, 123);
}

TEST(TestParser, ReadKeyValInteger) {
    std::string_view content = R"(
CoreDumping:      3
THP_enabled:    1
Threads:1
FDSize: 512
Name: init
VmSize: 100 kB
Empty:
)";
    auto check_integer = [&](const std::string& key, int value) {
        int val = 0;
        EXPECT_TRUE(ReadKeyVal(content, key, &val, false));
        EXPECT_EQ(val, value);
    };
    check_integer("CoreDumping:", 3);
    check_integer("THP_enabled:", 1);
    check_integer("Threads:", 1);
    check_integer("FDSize:", 512);

    // non-integer key
    int value = 0;
    EXPECT_FALSE(ReadKeyVal(content, "Name:", &value, true));
    EXPECT_FALSE(ReadKeyVal(content, "VmSize:", &value, true));
    EXPECT_FALSE(ReadKeyVal(content, "Empty:", &value, true));
}

TEST(TestParser, ReadKeyVal) {
    auto check = [&](const std::string& key, const std::string& value) {
        std::string val;
        EXPECT_TRUE(ReadKeyVal(status_content, key, &val, false));
        EXPECT_EQ(val, value);
    };

    check("Name:", "systemd");
    check("x86_Thread_features:", "");
    check("State:", "S (sleeping)");
    check("VmSize:", "23656 kB");
    check("Mems_allowed_list:", "0");
    check("voluntary_ctxt_switches:", "4408");
    check("Cpus_allowed:", "ffffffff,ffffffff,ffffffff,ffffffff");
    check("Uid:", "0       0       0       0");

    // non-exist key
    std::string value;
    EXPECT_FALSE(ReadKeyVal(status_content, "NonExistentKey:", &value, false));
}

TEST(TestParser, ReadKeyVal1) {
    std::string content = "avg10=0.5";
    float value = 0;
    EXPECT_TRUE(ReadKeyVal(content, "avg10=", &value, true));
    EXPECT_EQ(value, 0.5);
}