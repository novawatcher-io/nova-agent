#pragma once

#include "nova_agent_payload/node/v1/info.grpc.pb.h"

namespace App::Source::Host::Collector::Api {
class Collector {
public:
    virtual void run(novaagent::node::v1::NodeInfo* info) = 0;

    virtual ~Collector() = default;
};
} // namespace App::Source::Host::Collector::Api
