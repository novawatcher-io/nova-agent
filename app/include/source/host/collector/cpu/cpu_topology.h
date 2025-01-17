#pragma once
#include <string>

namespace App::Source::Host::Collector::Cpu {
class CPUTopology {
public:
    static void GetCPUTopology(std::string& topology);
    static std::string GetTopologyXML();
};
} // namespace App::Source::Host::Collector::Cpu
