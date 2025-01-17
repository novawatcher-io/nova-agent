
#include "app/include/source/host/collector/cpu/collector.h"
#include "app/include/source/host/collector/cpu/cpu_feature.h"
#include <cpu_features/cpu_features_macros.h>
#include <memory>

#if defined(CPU_FEATURES_ARCH_X86)
#include <cpu_features/cpuinfo_x86.h>
#elif defined(CPU_FEATURES_ARCH_ARM)
#include <cpu_features/cpuinfo_arm.h>
#elif defined(CPU_FEATURES_ARCH_AARCH64)
#include <cpu_features/cpuinfo_aarch64.h>
#elif defined(CPU_FEATURES_ARCH_MIPS)
#include <cpu_features/cpuinfo_mips.h>
#elif defined(CPU_FEATURES_ARCH_PPC)
#include <cpu_features/cpuinfo_ppc.h>
#elif defined(CPU_FEATURES_ARCH_S390X)
#include <cpu_features/cpuinfo_s390x.h>
#elif defined(CPU_FEATURES_ARCH_RISCV)
#include <cpu_features/cpuinfo_riscv.h>
#elif defined(CPU_FEATURES_ARCH_LOONGARCH)
#include <cpu_features/cpuinfo_loongarch.h>
#endif

namespace App::Source::Host::Collector::Cpu {

Collector::Collector() {
    cpuCollectors.push_back((std::make_unique<CpuFeature>()));
}

void Collector::run(deepagent::node::v1::NodeInfo* info) {
    for (auto& collector : cpuCollectors) {
        collector->run(info);
    }
}

void Collector::stop() {
}

} // namespace App::Source::Host::Collector::Cpu
