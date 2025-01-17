/**
******************************************************************************
* @file           : management_call_data.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/3/2
******************************************************************************
*/
//
// Created by zhanglei on 2024/3/2.
//
#pragma once

#include "skywalking-data-collect-protocol/management/Management.grpc.pb.h"

#include "app/include/api/call_data.h"

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
using namespace skywalking::v3;

class ManagementServerCallData : public Api::CallData {
public:
    ManagementServerCallData(grpc::ServerCompletionQueue* cq, ManagementService::AsyncService& service_)
        : cq_(cq), service(service_) {
        Proceed();
    }

    void onCreate() final {
        service.RequestkeepAlive(&ctx_, &request, &response, cq_, cq_, this);
    }

    void onProcess() override {
        new ManagementServerCallData(cq_, service);
        InstancePingPkg r;
        response.Finish(reply_, grpc::Status::OK, this);
        finish();
    }

    void onFinish() final {
    }

private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::ServerCompletionQueue* cq_;

    ManagementService::AsyncService& service;

    InstancePingPkg request;

    ::skywalking::v3::Commands reply_;

    ::grpc::ServerAsyncResponseWriter<Commands> response{&ctx_};

    ::grpc::ServerAsyncReader<Commands, InstancePingPkg> reader{&ctx_};
};

}
}
}
}