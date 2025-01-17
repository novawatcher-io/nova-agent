#pragma once

#include <component/api.h>
#include <skywalking-data-collect-protocol/logging/Logging.grpc.pb.h>

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
namespace Callback {

class LogReportServer :public skywalking::v3::LogReportService::Service {
public:
    LogReportServer(std::unique_ptr<Core::Component::Pipeline> logPipeline_) :logPipeline(std::move(logPipeline_)) {

    }

    ~LogReportServer() {

    }

    ::grpc::Status collect(::grpc::ServerContext* context, ::grpc::ServerReader< ::skywalking::v3::LogData>* reader, ::skywalking::v3::Commands* response) override ;

private:
    std::unique_ptr<Core::Component::Pipeline> logPipeline;
    skywalking::v3::LogData request;
};

}
}
}
}
}