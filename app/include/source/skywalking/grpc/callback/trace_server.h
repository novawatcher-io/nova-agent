#pragma once
#include <component/api.h>

#include <skywalking-data-collect-protocol/language-agent/Tracing.grpc.pb.h>
#include <iostream>

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
namespace Callback {
class TraceServer :public skywalking::v3::TraceSegmentReportService::Service {
public:
    TraceServer(std::unique_ptr<Core::Component::Pipeline> tracePipeline_) :tracePipeline(std::move(tracePipeline_)) {

    }

    ~TraceServer() {

    }
    // Recommended trace segment report channel.
    // gRPC streaming provides better performance.
    // All language agents should choose this.
    ::grpc::Status collect(::grpc::ServerContext* context, ::grpc::ServerReader< ::skywalking::v3::SegmentObject>* reader, ::skywalking::v3::Commands* response) override;

private:

    std::unique_ptr<Core::Component::Pipeline> tracePipeline;
};
}
}
}
}
}