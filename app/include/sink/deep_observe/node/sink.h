#pragma once

#include <memory>

#include "app/include/common/grpc/grpc_client_options.h"
#include "call_data.h"
#include "client.h"
#include "config/nova_agent_config.h"
#include "nova_agent_payload/node/v1/info.grpc.pb.h"
#include <component/api.h>

namespace App::Sink::DeepObserve::Node {
using namespace Core::Component;
class Sink : public Core::Component::Consumer {
public:
    Sink(std::shared_ptr<App::Config::ConfigReader> config) : config_(config) {
        Common::Grpc::ClientOptions options;
        options.max_concurrent_requests = 1000;
        options.endpoint = config_->NodeReportHost();
        options.metadata.insert({"company_uuid", config_->GetConfig().company_uuid()});
        client = std::make_unique<Client>(options);
    }

    Result Consume(std::shared_ptr<Batch>& /*batch*/) override {
        return {};
    }

    std::shared_ptr<Core::Component::Queue>& channel() final {
        return queue;
    }

    bool bindChannel(std::shared_ptr<Queue> /*channel*/) override {
        return false;
    };

    void stop() final {
    }

    Result Consume(Batch&) {
        return {};
    };

    Result send(novaagent::node::v1::NodeInfo& request);
    void SendUpdate(novaagent::node::v1::ProcessInfoRequest& request);
    void SendContainerInfo(novaagent::node::v1::ContainerInfoRequest& request);

    ~Sink() override = default;

private:
    std::shared_ptr<Core::Component::Queue> queue;
    std::unique_ptr<Client> client;
    std::shared_ptr<App::Config::ConfigReader> config_;
};
} // namespace App::Sink::DeepObserve::Node
