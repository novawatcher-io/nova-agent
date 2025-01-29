#include "app/include/source/host/collector/node/collector.h"
#include <algorithm>
#include <regex>
#include <string>
#include <sys/utsname.h>

#include "app/include/common/file.h"
#include "app/include/common/machine.h"
#include "os/unix_util.h"
#include "os/network_interface.h"

namespace App::Source::Host::Collector::Node {

using LSB = struct LSB_ {
    std::string ID;
    std::string Release;
    std::string Codename;
    std::string Description;
};

// 读取 /etc/lsb-release
LSB* getLSB(LSB* lsb) {
    if (lsb == nullptr) {
        return nullptr;
    }
    if (Common::IsFileExist("/etc/lsb-release")) {
        int fd = open("/etc/lsb-release", O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            return nullptr;
        }
        char lineBuf[BUFSIZ];
        while (Core::OS::readline(fd, lineBuf, BUFSIZ) > 0) {
            std::string_view str(lineBuf);
            auto pos = str.find('=');
            if (pos == std::string::npos) {
                continue;
            }
            auto key = str.substr(0, pos);

            auto begin = pos + 1;
            if (str.length() < begin) {
                continue;
            }
            int end = str.length() - begin - 1;
            if (end < 0) {
                continue;
            }
            auto value = str.substr(begin, end);

            if (key == "DISTRIB_ID") {
                lsb->ID = value;
            }

            if (key == "DISTRIB_RELEASE") {
                lsb->Release = value;
            }

            if (key == "DISTRIB_CODENAME") {
                lsb->Codename = value;
            }

            if (key == "DISTRIB_DESCRIPTION") {
                lsb->Description = value;
            }
        }

        close(fd);
        return lsb;
    } else {
        return nullptr;
    }
}

// 读取readhat版本号
std::string getRedhatishVersion(const std::vector<std::string>& contents) {
    std::string c;

    // 拼接字符串
    for (const auto& s : contents) {
        c += s;
    }

    // 转换为小写
    std::transform(c.begin(), c.end(), c.begin(), [](unsigned char c) { return std::tolower(c); });

    // 检查是否包含 "rawhide"
    if (c.find("rawhide") != std::string::npos) {
        return "rawhide";
    }

    // 使用正则表达式提取版本号
    std::regex re("release (\\d[\\d.]*)");
    std::smatch matches;
    if (std::regex_search(c, matches, re)) {
        return matches[1].str();
    }

    return "";
}

// 读取Slackware版本号
std::string getSlackwareVersion(const std::vector<std::string>& contents) {
    std::string c;

    // 拼接字符串
    for (const auto& s : contents) {
        c += s;
    }

    // 转换为小写
    std::transform(c.begin(), c.end(), c.begin(), [](unsigned char c) { return std::tolower(c); });

    // 替换第一个 "slackware "
    size_t pos = c.find("slackware ");
    if (pos != std::string::npos) {
        c.replace(pos, 10, ""); // 10 是 "slackware " 的长度
    }

    return c;
}

// 读取readhat paltform
std::string getRedhatishPlatform(const std::vector<std::string>& contents) {
    std::string c;

    // 拼接字符串
    for (const auto& s : contents) {
        c += s;
    }

    // 转换为小写
    std::transform(c.begin(), c.end(), c.begin(), [](unsigned char c) { return std::tolower(c); });

    // 检查是否包含 "red hat"
    if (c.find("red hat") != std::string::npos) {
        return "redhat";
    }

    // 按空格分割字符串
    std::istringstream iss(c);
    std::string firstWord;
    iss >> firstWord;

    return firstWord;
}

std::vector<std::string> readLines(const char* path) {
    std::vector<std::string> list;

    int fd = open("/etc/oracle-release", O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        return list;
    }
    char lineBuf[BUFSIZ];
    while (Core::OS::readline(fd, lineBuf, BUFSIZ)) {
        list.emplace_back(lineBuf);
    }

    close(fd);
    return list;
}

std::string getSuseVersion(const std::vector<std::string>& contents) {
    std::string version;

    for (const auto& line : contents) {
        // 使用正则表达式提取 VERSION
        std::regex reVersion("VERSION = ([\\d.]+)");
        std::smatch matchesVersion;
        if (std::regex_search(line, matchesVersion, reVersion)) {
            version = matchesVersion[1].str();
        }

        // 使用正则表达式提取 PATCHLEVEL
        std::regex rePatchlevel("PATCHLEVEL = ([\\d]+)");
        std::smatch matchesPatchlevel;
        if (std::regex_search(line, matchesPatchlevel, rePatchlevel)) {
            if (!version.empty()) {
                version += "." + matchesPatchlevel[1].str();
            }
        }
    }

    return version;
}

std::string getSusePlatform(const std::vector<std::string>& contents) {
    std::string c;

    // 拼接字符串
    for (const auto& s : contents) {
        c += s;
    }

    // 转换为小写
    std::transform(c.begin(), c.end(), c.begin(), [](unsigned char c) { return std::tolower(c); });

    // 检查是否包含 "opensuse"
    if (c.find("opensuse") != std::string::npos) {
        return "opensuse";
    }

    return "suse";
}

std::pair<std::string, std::string> getOSRelease() {
    auto contents = readLines("/etc/os-release");
    char name[128];
    char value[128];
    std::string platform;
    std::string version;
    for (auto& c : contents) {
        auto pos = c.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        auto key = c.substr(0, pos);

        auto begin = pos + 1;
        if (c.length() < begin) {
            continue;
        }
        int end = c.length() - begin - 1;
        if (end < 0) {
            continue;
        }
        auto value = c.substr(begin, end);

        if (key == "ID") {
            platform = value;
        }

        if (key == "VERSION_ID") {
            version = value;
        }
    }

    if (platform.empty()) {
        return {};
    }

    // cleanup amazon ID
    if (platform == "amzn") {
        platform = "amazon";
    }

    return {platform, version};
}

std::string getPlatform() {
    std::string platform = "unknown";
    std::string version = "unknown";
    LSB lsbValue;
    lsbValue.ID = platform;
    lsbValue.Release = version;
    LSB* lsb = getLSB(&lsbValue);
    if (lsb == nullptr) {
        lsb = &lsbValue;
    }
    if (Common::IsFileExist("/etc/oracle-release")) {
        platform = "oracle";
        auto content = readLines("/etc/oracle-release");
        version = getRedhatishVersion(content);
    } else if (Common::IsFileExist("/etc/enterprise-release")) {
        platform = "oracle";
        auto content = readLines("/etc/enterprise-release");
        version = getRedhatishVersion(content);
    } else if (Common::IsFileExist("/etc/slackware-version")) {
        platform = "slackware";
        auto content = readLines("/etc/enterprise-release");
        version = getSlackwareVersion(content);
    } else if (Common::IsFileExist("/etc/debian_version")) {
        if (lsb != nullptr) {
            if (lsb->ID == "Ubuntu") {
                platform = "ubuntu";
                version = lsb->Release;
            } else if (lsb->ID == "LinuxMint") {
                platform = "linuxmint";
                version = lsb->Release;
            } else {
                if (Common::IsFileExist("/usr/bin/raspi-config")) {
                    platform = "raspbian";
                } else {
                    platform = "debian";
                }
            }
        }
    } else if (Common::IsFileExist("/etc/redhat-release")) {
        auto content = readLines("/etc/redhat-release");
        platform = getRedhatishPlatform(content);
        version = getRedhatishVersion(content);
    } else if (Common::IsFileExist("/etc/system-release")) {
        auto content = readLines("/etc/redhat-release");
        platform = getRedhatishPlatform(content);
        version = getRedhatishVersion(content);
    } else if (Common::IsFileExist("/etc/gentoo-release")) {
        platform = "gentoo";
        auto content = readLines("/etc/gentoo-release");
        version = getRedhatishVersion(content);
    } else if (Common::IsFileExist("/etc/SuSE-release")) {
        auto content = readLines("/etc/SuSE-release");
        platform = getSusePlatform(content);
        version = getSuseVersion(content);
    } else if (Common::IsFileExist("/etc/arch-release")) {
        platform = "arch";
        version = lsb->Release;
    } else if (Common::IsFileExist("/etc/alpine-release")) {
        platform = "alpine";
        auto content = readLines("/etc/gentoo-release");
        if (!content.empty()) {
            version = content[0];
        }
    } else if (Common::IsFileExist("/etc/os-release")) {
        auto pair = getOSRelease();
        if (!pair.first.empty()) {
            platform = pair.first;
            version = pair.second;
        }
    } else if (lsb->ID == "RedHat") {
        platform = "redhat";
        version = lsb->Release;
    } else if (lsb->ID == "Amazon") {
        platform = "amazon";
        version = lsb->Release;
    } else if (lsb->ID == "ScientificSL") {
        platform = "scientific";
        version = lsb->Release;
    } else if (lsb->ID == "XenServer") {
        platform = "xenserver";
        version = lsb->Release;
    } else if (!lsb->ID.empty()) {
        // 转换为小写
        std::transform(lsb->ID.begin(), lsb->ID.end(), lsb->ID.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        platform = lsb->ID;
        version = lsb->Release;
    }
    return platform + " " + version;
}

void Collector::run(novaagent::node::v1::NodeInfo* info) {
    // 读取机器id
    info->set_host_object_id(Common::getMachineId());

    // 读取操node name
    utsname uts;
    uname(&uts);
    std::string hostname(uts.nodename);
    info->set_host_name(hostname);

    // 读取操作系统 systemos
    std::string sysname(uts.sysname);
    info->set_system_os(sysname);

    // 读取操作系统 版本
    std::string version(uts.version);
    info->set_system_version(sysname);

    // 读取操作系统类型，看是物理机还是虚拟机

    // 读取发行版本类型
    info->set_platform(getPlatform());

    // 获取IP地址
    Core::OS::IpInfo result;
    if (Core::OS::GetOneIpInfo(result)) {
        info->set_ipv4(result.ipv4);
        info->set_ipv6(result.ipv6);
    }
}
} // namespace App::Source::Host::Collector::Node
