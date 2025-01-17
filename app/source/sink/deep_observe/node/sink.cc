#include "app/include/sink/deep_observe/node/sink.h"
namespace App::Sink::DeepObserve::Node {
Result Sink::send(deepagent::node::v1::NodeInfo& data) {
    client->SendRegisterRequest(data);
    return {};
}
void Sink::SendUpdate(deepagent::node::v1::ProcessInfoRequest& request) {
    client->SendReportProcessRequest(request);
}

void Sink::SendContainerInfo(deepagent::node::v1::ContainerInfoRequest& request) {
    client->SendReportContainerRequest(request);
}
} // namespace App::Sink::DeepObserve::Node
