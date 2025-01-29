#pragma once
#include "app/include/source/host/collector/api/collector.h"
#include <string>

namespace App::Source::Host::Collector::Cpu {
class CpuFeature : public Api::Collector {
public:
    void run(novaagent::node::v1::NodeInfo* info) final;
    void stop();

private:
    // 是否运行
    bool isRun = false;
};
} // namespace App::Source::Host::Collector::Cpu
