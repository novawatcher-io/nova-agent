#pragma once
#include "app/include/source/host/collector/api/collector.h"
#include "cpu_usage.h"
#include "opentelemetry/metrics/meter.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include <cpu_features/cpu_features_macros.h>
#include <memory>
#include <vector>

namespace novaagent {
namespace node {
namespace v1 {
class NodeInfo;
}
} // namespace node
} // namespace novaagent

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
#include "app/include/source/host/collector/api/collector.h"
#include "cpu_usage.h"
#include "cpuinfo.h"

namespace App {
namespace Source {
namespace Host {
namespace Collector {
namespace Cpu {
class Collector : public Api::Collector {
public:
    Collector();

    void run(novaagent::node::v1::NodeInfo* info) final;

    void install(novaagent::node::v1::NodeInfo* info) final {};

    void start() {};

    void stop() final;

private:
    std::vector<std::unique_ptr<Api::Collector>> cpuCollectors;
    opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter;
};
} // namespace Cpu
} // namespace Collector
} // namespace Host
} // namespace Source
} // namespace App
