#include "app/include/source/host/collector/cpu/cpu_feature.h"

extern "C" {
#include <unistd.h>
}

#include <string>

#include <chrono>
#include <functional>
#include <iostream>

#include <fmt/format.h>

#include <cpu_features/cpu_features_macros.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/sdk/common/attribute_utils.h>
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
#include "app/include/source/host/collector/cpu/const.h"

namespace App::Source::Host::Collector::Cpu {
using namespace cpu_features;

#define DEFINE_ADD_FLAGS(HasFeature, FeatureName, FeatureType, LastEnum)                            \
    static void AddFlags(deepagent::node::v1::CpuFeature* feature, const FeatureType* features) {   \
        size_t i;                                                                                   \
        const char* ptrs[LastEnum] = {0};                                                           \
        for (i = 0; i < LastEnum; ++i) {                                                            \
            if (HasFeature(features, static_cast<cpu_features::X86FeaturesEnum>(i))) {              \
                *feature->add_flags() = FeatureName(static_cast<cpu_features::X86FeaturesEnum>(i)); \
            }                                                                                       \
        }                                                                                           \
    }

#if defined(CPU_FEATURES_ARCH_X86)
DEFINE_ADD_FLAGS(GetX86FeaturesEnumValue, GetX86FeaturesEnumName, X86Features, X86_LAST_)
#elif defined(CPU_FEATURES_ARCH_ARM)
DEFINE_ADD_FLAGS(GetArmFeaturesEnumValue, GetArmFeaturesEnumName, ArmFeatures, ARM_LAST_)
#elif defined(CPU_FEATURES_ARCH_AARCH64)
DEFINE_ADD_FLAGS(GetAarch64FeaturesEnumValue, GetAarch64FeaturesEnumName, Aarch64Features, AARCH64_LAST_)
#elif defined(CPU_FEATURES_ARCH_MIPS)
DEFINE_ADD_FLAGS(GetMipsFeaturesEnumValue, GetMipsFeaturesEnumName, MipsFeatures, MIPS_LAST_)
#elif defined(CPU_FEATURES_ARCH_PPC)
DEFINE_ADD_FLAGS(GetPPCFeaturesEnumValue, GetPPCFeaturesEnumName, PPCFeatures, PPC_LAST_)
#elif defined(CPU_FEATURES_ARCH_S390X)
DEFINE_ADD_FLAGS(GetS390XFeaturesEnumValue, GetS390XFeaturesEnumName, S390XFeatures, S390X_LAST_)
#elif defined(CPU_FEATURES_ARCH_RISCV)
DEFINE_ADD_FLAGS(GetRiscvFeaturesEnumValue, GetRiscvFeaturesEnumName, RiscvFeatures, RISCV_LAST_)
#elif defined(CPU_FEATURES_ARCH_LOONGARCH)
DEFINE_ADD_FLAGS(GetLoongArchFeaturesEnumValue, GetLoongArchFeaturesEnumName, LoongArchFeatures, LOONGARCH_LAST_)
#endif

static std::string GetCacheTypeString(CacheType cache_type) {
    switch (cache_type) {
    case CPU_FEATURE_CACHE_NULL:
        return ("null");
    case CPU_FEATURE_CACHE_DATA:
        return ("data");
    case CPU_FEATURE_CACHE_INSTRUCTION:
        return ("instruction");
    case CPU_FEATURE_CACHE_UNIFIED:
        return ("unified");
    case CPU_FEATURE_CACHE_TLB:
        return ("tlb");
    case CPU_FEATURE_CACHE_DTLB:
        return ("dtlb");
    case CPU_FEATURE_CACHE_STLB:
        return ("stlb");
    case CPU_FEATURE_CACHE_PREFETCH:
        return ("prefetch");
    }
    CPU_FEATURES_UNREACHABLE();
}

static void AddCacheInfo(deepagent::node::v1::CpuFeature* feature, const cpu_features::CacheInfo* cache) {
    for (int i = 0; i < cache->size; ++i) {
        CacheLevelInfo info = cache->levels[i];
        deepagent::node::v1::CpuFeatureCacheKeyValue* data = feature->add_caches();
        ::google::protobuf::Map<std::string, std::string>* cacheMap = data->mutable_values();
        (*cacheMap)[level] = std::to_string(info.level);
        (*cacheMap)[cache_type] = GetCacheTypeString(info.cache_type);
        (*cacheMap)[ways] = std::to_string(info.ways);
        (*cacheMap)[cache_size] = std::to_string(info.cache_size);
        (*cacheMap)[line_size] = std::to_string(info.line_size);
        (*cacheMap)[tlb_entries] = std::to_string(info.tlb_entries);
        (*cacheMap)[partitioning] = std::to_string(info.partitioning);
    }
}

// 最后采集时间
static std::chrono::steady_clock::time_point lastCollectTime = std::chrono::steady_clock::time_point::min();

void CpuFeature::run(deepagent::node::v1::NodeInfo* data) {
    if (isRun) {
        return;
    }

    std::map<std::string, std::string> labels;
    opentelemetry::sdk::common::AttributeMap resourceAttr;
    if (lastCollectTime != std::chrono::steady_clock::time_point::min()) {
        auto nowTime = std::chrono::steady_clock::now();
        auto duration = nowTime - lastCollectTime;
        long long elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        if (elapsed_seconds < 3600) {
            return;
        }
    }

    // 检查是否到达要采集时间，如果不到达则不进行收集
#if defined(CPU_FEATURES_ARCH_X86)
    const ::cpu_features::X86Info info = ::cpu_features::GetX86Info();
    const ::cpu_features::CacheInfo cache = ::cpu_features::GetX86CacheInfo();
    auto feature = new deepagent::node::v1::CpuFeature();
    std::string arch_str("x86");
    feature->set_arch(arch_str);
    std::string brand_string(info.brand_string);
    feature->set_brand(brand_string);
    std::string family_str = std::to_string(info.family);
    feature->set_family(family_str);
    auto model_Str = std::to_string(info.model);
    feature->set_model(model_Str);
    auto stepping_str = std::to_string(info.stepping);
    feature->set_stepping(stepping_str);
    std::string uarch_str(GetX86MicroarchitectureName(GetX86Microarchitecture(&info)));
    feature->set_uarch(uarch_str);
    AddFlags(feature, &info.features);
    AddCacheInfo(feature, &cache);
    data->set_allocated_cpu_feature(feature);
#elif defined(CPU_FEATURES_ARCH_ARM)
    const ArmInfo info = GetArmInfo();
    labels[arch] = "ARM";
    labels[implementer] = info.implementer;
    labels[architecture] = std::to_string(info.architecture);
    labels[variant] = std::to_string(info.variant);
    labels[part] = std::to_string(info.part);
    labels[revision] = std::to_string(info.revision);
    labels[uarch] = GetX86MicroarchitectureName(GetX86Microarchitecture(&info));
    AddFlags(labels, &info.features);
#elif defined(CPU_FEATURES_ARCH_AARCH64)
    const Aarch64Info info = GetAarch64Info();
    labels[arch] = "aarch64";
    labels[implementer] = info.implementer;
    labels[variant] = std::to_string(info.variant);
    labels[part] = std::to_string(info.part);
    labels[revision] = std::to_string(info.revision);
    AddFlags(labels, &info.features);
#elif defined(CPU_FEATURES_ARCH_MIPS)
    const MipsInfo info = GetMipsInfo();
    labels[arch] = "mips";
    AddFlags(labels, &info.features);
#elif defined(CPU_FEATURES_ARCH_PPC)
    const PPCInfo info = GetPPCInfo();
    const PPCPlatformStrings strings = GetPPCPlatformStrings();
    labels[arch] = "ppc";
    labels[platform] = info.platform;
    labels[model] = std::to_string(info.model);
    labels[machine] = std::to_string(info.machine);
    labels[cpu] = std::to_string(info.cpu);
    labels[instruction] = strings.type.platform;
    labels[microarchitecture] = std::to_string(strings.type.base_platform);
    AddFlags(labels, &info.features);

#elif defined(CPU_FEATURES_ARCH_S390X)
    const S390XInfo info = GetS390XInfo();
    const S390XPlatformStrings strings = GetS390XPlatformStrings();
    labels[arch] = "s390x";
    labels[platform] = "zSeries";
    labels[model] = std::to_string(strings.type.platform);
    labels[processors] = std::to_string(strings.num_processors);
    labels[cpu] = std::to_string(info.cpu);
    labels[instruction] = strings.type.platform;
    labels[microarchitecture] = std::to_string(strings.type.base_platform);
    AddFlags(labels, &info.features);
#elif defined(CPU_FEATURES_ARCH_RISCV)
    const RiscvInfo info = GetRiscvInfo();
    labels[arch] = "risc-v";
    labels[vendor] = std::string(info.vendor);
    labels[microarchitecture] = std::string(info.uarch);
    AddFlags(labels, &info.features);
#elif defined(CPU_FEATURES_ARCH_LOONGARCH)
    const LoongArchInfo info = GetLoongArchInfo();
    labels[arch] = "loongarch";
    AddFlags(labels, &info.features);
#endif
}

void CpuFeature::stop() {
    isRun = false;
}
} // namespace App::Source::Host::Collector::Cpu
