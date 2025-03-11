//
// Created by root on 2024/7/28.
//
#pragma once

extern "C" {
#include <ifaddrs.h>
#include <net/if.h>
#include <unistd.h>
#include <cpuid.h>
#include <sys/sysinfo.h>
#include <linux/if_packet.h>
}

#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include "common/xxhash32.h"
#include "common/xxhash64.h"

namespace App {
namespace Common {
static std::mutex mtx;
static uint64_t objectId = 0;
//  读取主机id，用来区分不同物理机，先读取/etc/machineid，如果读不到则把cpuid和物理网卡地址做xxhash
inline static uint64_t getMachineId() {
    if (objectId > 0) {
        return objectId;
    }
    std::lock_guard guard(mtx);
    if (objectId > 0) {
        return objectId;
    }
    // 优先读取 /etc/machine-id 文件
    Core::Common::XXHash64 hashUtil(0);
    std::ifstream machine_id_file("/etc/machine-id");
    if (machine_id_file.is_open()) {
        std::string machine_id;
        std::getline(machine_id_file, machine_id);
        machine_id_file.close();
        hashUtil.add(machine_id.data(), machine_id.length());
        objectId = hashUtil.hash();
        return objectId;
    }

    // 如果无法读取 /etc/machine-id 文件，则尝试获取 CPUID 和物理网卡地址
    unsigned int cpu_info[4];
    __cpuid(cpu_info, cpu_info[0], cpu_info[1], cpu_info[2], cpu_info[3]);
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0')
     << cpu_info[1] << cpu_info[3] << cpu_info[2] << cpu_info[3];

     // 获取物理网卡地址
    struct ifaddrs *ifaddr, *ifa;
    getifaddrs(&ifaddr);
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            struct sockaddr_ll *sll = (struct sockaddr_ll *)ifa->ifa_addr;
            unsigned char mac_address[6] = {0};
            memcpy(mac_address, sll->sll_addr, 6);
            ss << "-" << std::hex << std::setw(2) << std::setfill('0')
             << static_cast<int>(mac_address[0])
             << static_cast<int>(mac_address[1])
             << ":" << std::hex << std::setw(2) << std::setfill('0')
             << static_cast<int>(mac_address[2])
             << static_cast<int>(mac_address[3])
             << ":" << std::hex << std::setw(2) << std::setfill('0')
             << static_cast<int>(mac_address[4])
             << static_cast<int>(mac_address[5]);
    }
    freeifaddrs(ifaddr);
    hashUtil.add(ss.str().data(), ss.str().length());
    objectId = hashUtil.hash();
    return objectId;
}
}
}
