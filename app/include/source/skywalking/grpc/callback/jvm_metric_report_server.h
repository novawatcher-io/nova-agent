#pragma once
#include "trace_server.h"

#include <skywalking-data-collect-protocol/language-agent/JVMMetric.grpc.pb.h>
#include <iostream>

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
namespace Callback {
class JvmMetricReportServer :public skywalking::v3::JVMMetricReportService::Service {
public:
    JvmMetricReportServer(std::unique_ptr<Core::Component::Pipeline> metricPipeline_) :metricPipeline(std::move(metricPipeline_)) {

    }

    ~JvmMetricReportServer() {

    }

    ::grpc::Status collect(::grpc::ServerContext* context, const ::skywalking::v3::JVMMetricCollection* request, ::skywalking::v3::Commands* response) override;

private:
    std::unique_ptr<Core::Component::Pipeline> metricPipeline;

};
}
}
}
}
}