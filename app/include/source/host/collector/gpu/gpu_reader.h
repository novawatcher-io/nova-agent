#pragma once
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#ifdef ENABLE_GPU
#include <nvml.h>
#endif
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace App::Source::Host::Collector::GPU {
struct GPUProcessUsage {
    unsigned int pid = 0;
    unsigned int memory = 0;
    unsigned int sm_util = 0;
    unsigned int enc_util = 0;
    unsigned int dec_util = 0;
};

using App::Source::Host::Collector::Oltp::MultiValue;

#ifndef ENABLE_GPU
using nvmlDevice_t = void*;
#endif

/**
 * GPUReader class is used to read GPU information.
 * There maybe more than one GPU on a machine, so we need to get the usage of each GPU.
 */
class GPUReader {
public:
    GPUReader();
    ~GPUReader();
    bool IsGPUAvailable() const;
    size_t GetGPUCount() const;

    void GetGPUUsage(MultiValue& values);
    void GetGPUMemoryUsage(MultiValue& values);
    void GetGPUMemoryUsed(MultiValue& values);
    void GetGPUMemoryFree(MultiValue& values);
    void GetGPUTemperature(MultiValue& values);
    void GetGPUPowerDraw(MultiValue& values);
    void GetGPUCoreClock(MultiValue& values);
    void GetGPUFanSpeed(MultiValue& values);
    void GetGPUProcessMemoryUsage(MultiValue& values);
    void GetGPUProcessMemoryUsage(std::unordered_map<int, GPUProcessUsage>& usage);
    void GetGPUPCIERXBandwidth(MultiValue& values);
    void GetGPUPCIETXBandwidth(MultiValue& values);
    void GetGPUEccErrors(MultiValue& values);

    void GetGPUData(MultiValue& values, std::function<std::variant<int64_t, double>(nvmlDevice_t device)> func);

private:
    std::vector<std::string> device_uuid_;
    std::vector<nvmlDevice_t> devices_;
    unsigned long long last_sample_ts_ = 0;
};
} // namespace App::Source::Host::Collector::GPU
