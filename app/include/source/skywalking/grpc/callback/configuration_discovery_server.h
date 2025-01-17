#pragma once

#include <skywalking-data-collect-protocol/language-agent/ConfigurationDiscoveryService.grpc.pb.h>

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
namespace Callback {
class ConfigurationDiscoveryServer :public ::skywalking::v3::ConfigurationDiscoveryService::Service {
public:
    ::grpc::Status fetchConfigurations(::grpc::ServerContext* , const ::skywalking::v3::ConfigurationSyncRequest* , ::skywalking::v3::Commands* ) {
        return ::grpc::Status::OK;
    }
};
}
}
}
}
}