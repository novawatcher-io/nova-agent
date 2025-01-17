#pragma once

#include <skywalking-data-collect-protocol/management/Management.grpc.pb.h>

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
namespace Callback {

class ManagementServer :public skywalking::v3::ManagementService::Service {
public:
    ::grpc::Status reportInstanceProperties(::grpc::ServerContext* , const ::skywalking::v3::InstanceProperties* , ::skywalking::v3::Commands* ) {
        return ::grpc::Status::OK;
    }
    // Keep the instance alive in the backend analysis.
    // Only recommend to do separate keepAlive report when no trace and metrics needs to be reported.
    // Otherwise, it is duplicated.
    ::grpc::Status keepAlive(::grpc::ServerContext* , const ::skywalking::v3::InstancePingPkg* , ::skywalking::v3::Commands* ) {
        return ::grpc::Status::OK;
    }

};

}
}
}
}
}