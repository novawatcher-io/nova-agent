#include "app/include/source/host/collector/gpu/gpu_reader.h"
#include "app/include/source/host/collector/oltp/oltp_metric.h"
#include <chrono>
#include <functional>
#include <map>
#ifdef ENABLE_GPU
#include <nvml.h>
#endif
#include <spdlog/spdlog.h>
#include <unordered_map>

using App::Source::Host::Collector::Oltp::MetricData;
namespace App::Source::Host::Collector::GPU {

GPUReader::GPUReader() {
#ifdef ENABLE_GPU
    // init gpu
    if (nvmlReturn_t const result = nvmlInit(); result != NVML_SUCCESS) {
        SPDLOG_ERROR("Failed to initialize NVML: {}", nvmlErrorString(result));
        return;
    }

    // get device count
    unsigned int device_count = 0;
    if (nvmlReturn_t const result = nvmlDeviceGetCount(&device_count); result != NVML_SUCCESS) {
        SPDLOG_ERROR("Failed to get device count: {}", nvmlErrorString(result));
        return;
    }
    if (device_count == 0) {
        SPDLOG_INFO("No devices found");
        return;
    }
    SPDLOG_INFO("devices count: {}", device_count);
    // official document says that the buffer size at most 96 bytes, give it enough space
    char buffer[128];
    devices_.resize(device_count);
    device_uuid_.resize(device_count);
    for (unsigned int i = 0; i < device_count; i++) {
        if (nvmlReturn_t const result = nvmlDeviceGetHandleByIndex(i, &devices_[i]); result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get device handle: {}", nvmlErrorString(result));
            continue;
        }

        if (nvmlReturn_t const result = nvmlDeviceGetUUID(devices_[i], buffer, 128); result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get device UUID: {}", nvmlErrorString(result));
            continue;
        }
        SPDLOG_INFO("GPU {} UUID is: {}", i, buffer);
        device_uuid_[i] = buffer;
    }
#endif
}

GPUReader::~GPUReader() {
#ifdef ENABLE_GPU
    nvmlShutdown();
#endif
}

bool GPUReader::IsGPUAvailable() const {
    return !devices_.empty();
}

size_t GPUReader::GetGPUCount() const {
    return devices_.size();
}

void GPUReader::GetGPUData(MultiValue& values, std::function<std::variant<int64_t, double>(nvmlDevice_t device)> func) {
#ifdef ENABLE_GPU
    values.resize(devices_.size());
    for (size_t i = 0; i < devices_.size(); i++) {
        auto* device = devices_[i];
        values[i].data = func(device);
        values[i].labels["gpu_uuid"] = device_uuid_[i];
    }
#endif
}

void GPUReader::GetGPUUsage(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        nvmlUtilization_t utilization;
        auto result = nvmlDeviceGetUtilizationRates(device, &utilization);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get device utilization: {}", nvmlErrorString(result));
            return 0;
        }
        SPDLOG_INFO("GPU utilization: {}", utilization.gpu);
        return utilization.gpu;
    });
#endif
}

void GPUReader::GetGPUMemoryUsage(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        nvmlUtilization_t utilization;
        auto result = nvmlDeviceGetUtilizationRates(device, &utilization);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get device utilization: {}", nvmlErrorString(result));
            return 0;
        }
        return utilization.memory;
    });
#endif
}

void GPUReader::GetGPUMemoryUsed(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        nvmlMemory_t memory;
        auto result = nvmlDeviceGetMemoryInfo(device, &memory);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get memory info for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(memory.used);
    });
#endif
}

void GPUReader::GetGPUMemoryFree(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        nvmlMemory_t memory;
        auto result = nvmlDeviceGetMemoryInfo(device, &memory);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get memory info for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(memory.free);
    });
#endif
}

void GPUReader::GetGPUTemperature(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        unsigned int temperature = 0;
        auto result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get temperature for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(temperature);
    });
#endif
}

void GPUReader::GetGPUPowerDraw(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        unsigned int power_draw = 0;
        auto result = nvmlDeviceGetPowerUsage(device, &power_draw);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get power usage for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(power_draw);
    });
#endif
}

void GPUReader::GetGPUCoreClock(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        unsigned int clock = 0;
        auto result = nvmlDeviceGetClockInfo(device, NVML_CLOCK_GRAPHICS, &clock);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get clock info for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(clock);
    });
#endif
}

void GPUReader::GetGPUFanSpeed(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        unsigned int speed = 0;
        auto result = nvmlDeviceGetFanSpeed(device, &speed);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get fan speed for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(speed);
    });
#endif
}

void GPUReader::GetGPUProcessMemoryUsage(MultiValue& values) {
#ifdef ENABLE_GPU
    for (size_t i = 0; i < devices_.size(); i++) {
        auto device = devices_[i];
        std::vector<nvmlProcessInfo_t> infos(1024);
        bool try_again = true;
        while (try_again) {
            unsigned int info_count = static_cast<unsigned int>(infos.size());
            auto result = nvmlDeviceGetComputeRunningProcesses(device, &info_count, infos.data());
            if (result == NVML_SUCCESS) {
                for (unsigned int j = 0; j < info_count; j++) {
                    const auto& info = infos[i];
                    values.push_back(MetricData());
                    auto& value = values.back();
                    value.data = static_cast<int64_t>(info.usedGpuMemory);
                    value.labels["gpu_uuid"] = device_uuid_[i];
                    value.labels["pid"] = std::to_string(info.pid);
                    SPDLOG_INFO("Process {} memory usage: {}", info.pid, info.usedGpuMemory);
                }
                try_again = false;
            } else if (result == NVML_ERROR_INSUFFICIENT_SIZE) {
                const auto new_size = infos.size() * 2;
                infos.resize(new_size);
            } else {
                SPDLOG_ERROR("Failed to get active processes for device: {}", nvmlErrorString(result));
                try_again = false;
            }
        }
    }
#endif
}

void GPUReader::GetGPUProcessMemoryUsage(std::unordered_map<int, GPUProcessUsage>& usage) {
#ifdef ENABLE_GPU
    usage.clear();
    for (size_t i = 0; i < devices_.size(); i++) {
        auto* device = devices_[i];
        std::vector<nvmlProcessInfo_t> infos(1024);
        while (true) {
            auto info_count = static_cast<unsigned int>(infos.size());
            auto result = nvmlDeviceGetComputeRunningProcesses(device, &info_count, infos.data());
            if (result == NVML_SUCCESS) {
                for (unsigned int j = 0; j < info_count; j++) {
                    const auto& info = infos[i];
                    usage[info.pid].memory = info.usedGpuMemory;
                }
                break;
            } else if (result == NVML_ERROR_INSUFFICIENT_SIZE) {
                const auto new_size = infos.size() * 2;
                infos.resize(new_size);
            } else {
                SPDLOG_ERROR("Failed to get active processes for device: {}", nvmlErrorString(result));
                break;
            }
        }

        std::vector<nvmlProcessUtilizationSample_t> utils(1024);
        while (true) {
            unsigned int util_count = utils.size();
            auto result = nvmlDeviceGetProcessUtilization(device, utils.data(), &util_count, last_sample_ts_);
            if (result == NVML_SUCCESS) {
                for (unsigned int j = 0; j < util_count; j++) {
                    const auto& util = utils[j];
                    usage[util.pid].pid = util.pid;
                    usage[util.pid].sm_util = util.smUtil;
                    usage[util.pid].enc_util = util.encUtil;
                    usage[util.pid].dec_util = util.decUtil;
                }
                // get current ts in microsecond
                last_sample_ts_ = std::chrono::duration_cast<std::chrono::microseconds>(
                                      std::chrono::system_clock::now().time_since_epoch())
                                      .count();
                break;
            } else if (result == NVML_ERROR_INSUFFICIENT_SIZE) {
                const auto new_size = utils.size() * 2;
                utils.resize(new_size);
            } else {
                SPDLOG_ERROR("Failed to get process utilization for device: {}", nvmlErrorString(result));
                break;
            }
        }
    }
#endif
}

void GPUReader::GetGPUPCIERXBandwidth(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        unsigned int bandwidth;
        auto result = nvmlDeviceGetPcieThroughput(device, NVML_PCIE_UTIL_RX_BYTES, &bandwidth);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get PCIe RX bandwidth for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(bandwidth);
    });
#endif
}

void GPUReader::GetGPUPCIETXBandwidth(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        unsigned int bandwidth;
        auto result = nvmlDeviceGetPcieThroughput(device, NVML_PCIE_UTIL_TX_BYTES, &bandwidth);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get PCIe TX bandwidth for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(bandwidth);
    });
#endif
}

void GPUReader::GetGPUEccErrors(MultiValue& values) {
#ifdef ENABLE_GPU
    GetGPUData(values, [](nvmlDevice_t device) -> int64_t {
        unsigned long long ecc_errors;
        auto result =
            nvmlDeviceGetTotalEccErrors(device, NVML_MEMORY_ERROR_TYPE_CORRECTED, NVML_VOLATILE_ECC, &ecc_errors);
        if (result != NVML_SUCCESS) {
            SPDLOG_ERROR("Failed to get ECC errors for device: {}", nvmlErrorString(result));
            return 0;
        }
        return static_cast<int64_t>(ecc_errors);
    });
#endif
}
} // namespace App::Source::Host::Collector::GPU
