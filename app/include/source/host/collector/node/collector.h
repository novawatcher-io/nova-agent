#pragma once

#include "app/include/source/host/collector/api/collector.h"
#include "os/network_interface.h"

namespace App::Source::Host::Collector::Node {
class Collector : public Api::Collector {
public:
    void run(deepagent::node::v1::NodeInfo* info) final;

private:
    Core::OS::IpInfo ip_info_;
};
} // namespace App::Source::Host::Collector::Node
