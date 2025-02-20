//
// Created by zhanglei on 2025/2/19.
//

#pragma once

#include "app/include/common/grpc/grpc_client_options.h"

#include <nova_agent_payload/trace/v1/topology.grpc.pb.h>
#include <atomic>

namespace App::Sink::Topology {
class Sink {
public:
    explicit Sink(Common::Grpc::ClientOptions options);

    void registerService(novaagent::trace::v1::Service& service);

    void registerServiceRelation(novaagent::trace::v1::ServiceRelation& service);
private:
    Common::Grpc::ClientOptions options_;
    std::unique_ptr<novaagent::trace::v1::TraceTopologyCollectorService::Stub> stub_;
    std::atomic<std::size_t> running_requests{0};
    uint64_t max_concurrent_requests = 0;
};
}