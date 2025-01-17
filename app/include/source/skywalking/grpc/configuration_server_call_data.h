/**
******************************************************************************
* @file           : configuration_server.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/2/27
******************************************************************************
*/
//
// Created by zhanglei on 2024/2/27.
//
#pragma once

#include "app/include/api/call_data.h"

#include "skywalking-data-collect-protocol/language-agent/ConfigurationDiscoveryService.grpc.pb.h"

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
using namespace skywalking::v3;

class ConfigurationDiscoveryServiceCallData : public Api::CallData {
public:
    ConfigurationDiscoveryServiceCallData(grpc::ServerCompletionQueue* cq,
                                          ConfigurationDiscoveryService::AsyncService& service_)
        : cq_(cq), service(service_) {
    }

    void onCreate() {
        service.RequestfetchConfigurations(&ctx_, &request, &response, cq_, cq_, this);
    }

    void onProcess() final;

    void onFinish() final {
    }

    virtual ~ConfigurationDiscoveryServiceCallData() {
    }

private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::ServerCompletionQueue* cq_;

    ConfigurationDiscoveryService::AsyncService& service;

    ConfigurationSyncRequest request;

    ::grpc::ServerAsyncResponseWriter<Commands> response{&ctx_};
};
} // namespace Grpc
} // namespace SkyWalking
} // namespace Source
} // namespace App