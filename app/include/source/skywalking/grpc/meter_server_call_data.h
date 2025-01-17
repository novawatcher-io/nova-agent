/**
******************************************************************************
* @file           : metric_server.h
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

#include "skywalking-data-collect-protocol/language-agent/Meter.grpc.pb.h"

#include "app/include/api/call_data.h"

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
class MeterServerCallData : public Api::CallData {
public:
    MeterServerCallData(grpc::ServerCompletionQueue* cq, MeterReportService::AsyncService& service_)
        : cq_(cq), service(service_) {
    }

    void onCreate() final {
        service.RequestcollectBatch(&ctx_, &reader, cq_, cq_, this);
    }

    void onProcess() final {
    }

    void onFinish() final {
    }

    ~MeterServerCallData() {
    }

private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::ServerCompletionQueue* cq_;

    MeterReportService::AsyncService& service;

    ::grpc::ServerAsyncReader<Commands, MeterDataCollection> reader{&ctx_};
};
} // namespace Grpc
} // namespace SkyWalking
} // namespace Source
} // namespace App