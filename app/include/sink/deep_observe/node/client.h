#pragma once
#include "app/include/common/grpc/grpc_client_options.h"
#include "nova_agent_payload/node/v1/info.grpc.pb.h"
#include <memory>

namespace novaagent::node::v1 {
class NodeInfo;
class NodeUsage;
class ProcessInfoRequest;
} // namespace novaagent::node::v1

namespace App::Sink::DeepObserve::Node {

class Client {
public:
    explicit Client(Common::Grpc::ClientOptions options);

    void SendRegisterRequest(novaagent::node::v1::NodeInfo& request);
    void SendUpdateRequest(novaagent::node::v1::NodeUsage& request);
    void SendReportProcessRequest(novaagent::node::v1::ProcessInfoRequest& request);
    void SendReportContainerRequest(novaagent::node::v1::ContainerInfoRequest& request);

private:
    Common::Grpc::ClientOptions options_;
    std::unique_ptr<novaagent::node::v1::NodeCollectorService::Stub> stub_;
};
} // namespace App::Sink::DeepObserve::Node
