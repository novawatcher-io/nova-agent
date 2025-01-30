#pragma once

#include "app/include/source/host/collector/api/collector.h"

#include <string>

#include "os/network_interface.h"

namespace App::Source::Host::Collector::Node {
class Collector : public Api::Collector {
public:
    Collector();
    ~Collector() = default;
    void run(novaagent::node::v1::NodeInfo* info) final;

    uint64_t host_object_id_;
    std::string host_name_;
    std::string sysname_;
    std::string version_;
private:
    Core::OS::IpInfo ip_info_;
};
} // namespace App::Source::Host::Collector::Node
