//
// Created by zhanglei on 25-1-25.
//
#pragma once

#include "app/include/source/host/collector/api/collector.h"
#include "nova_agent_payload/node/v1/info.grpc.pb.h"

namespace App::Source::Host::Collector::Fs {
class FsCollector : public Api::Collector {
public:
    void run(novaagent::node::v1::NodeInfo* info);

    virtual ~FsCollector() = default;
};
}