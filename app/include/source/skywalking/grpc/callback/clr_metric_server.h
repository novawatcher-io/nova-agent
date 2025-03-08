//
// Created by zhanglei on 2025/3/6.
//

#pragma once

#include <skywalking-data-collect-protocol/language-agent/CLRMetric.grpc.pb.h>

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
namespace Callback {
class CLRMetricServer :public ::skywalking::v3::CLRMetricReportService::Service {
public:
    ::grpc::Status collect(::grpc::ServerContext* context, const ::skywalking::v3::CLRMetricCollection* request, ::skywalking::v3::Commands* response) {
        return ::grpc::Status::OK;
    }
};
}
}
}
}
}
