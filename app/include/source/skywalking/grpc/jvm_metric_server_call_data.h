/**
******************************************************************************
* @file           : jvm_metric_server.h
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

#include <skywalking-data-collect-protocol/language-agent/JVMMetric.grpc.pb.h>

#include "app/include/api/call_data.h"
#include "app/include/sink/channel/grpc_channel.h"

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
using grpc::ServerBuilder;
using namespace skywalking::v3;

class JvmMetricServerCallData : public Api::CallData {
public:
    JvmMetricServerCallData(grpc::ServerCompletionQueue* cq, JVMMetricReportService::AsyncService& service_,
                            std::unique_ptr<Core::Component::Pipeline>& channel)
        : cq_(cq), service(service_), pipeline_(channel) {
        Proceed();
    }

    void onCreate() final {
        service.Requestcollect(&ctx_, &request, &response, cq_, cq_, this);
    }

    void onProcess();

    void onFinish() final {
    }

    ~JvmMetricServerCallData() {
    }

private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::ServerCompletionQueue *cq_;

    JVMMetricReportService::AsyncService &service;

    ::skywalking::v3::JVMMetricCollection request;

    ::grpc::ServerAsyncResponseWriter<::skywalking::v3::Commands> response{&ctx_};

    ::skywalking::v3::Commands reply_;

    int times = 0;

    std::unique_ptr<Core::Component::Pipeline> &pipeline_;
};
}
}
}
}
