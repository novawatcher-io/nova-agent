#include "app/include/common/grpc/grpc_client_options.h"
#include "deep_agent_payload/node/v1/info.grpc.pb.h"
#include <memory>

namespace deepagent::node::v1 {
class NodeInfo;
class NodeUsage;
class ProcessInfoRequest;
} // namespace deepagent::node::v1

namespace App::Sink::DeepObserve::Node {

class Client {
public:
    explicit Client(Common::Grpc::ClientOptions options);

    void SendRegisterRequest(deepagent::node::v1::NodeInfo& request);
    void SendUpdateRequest(deepagent::node::v1::NodeUsage& request);
    void SendReportProcessRequest(deepagent::node::v1::ProcessInfoRequest& request);
    void SendReportContainerRequest(deepagent::node::v1::ContainerInfoRequest& request);

private:
    Common::Grpc::ClientOptions options_;
    std::unique_ptr<deepagent::node::v1::NodeCollectorService::Stub> stub_;
};
} // namespace App::Sink::DeepObserve::Node
