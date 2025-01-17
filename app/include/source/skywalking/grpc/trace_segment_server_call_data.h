/**
******************************************************************************
* @file           : server.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/2/20
******************************************************************************
*/
//
// Created by zhanglei on 2024/2/20.
//

#pragma once

#include <skywalking-data-collect-protocol/language-agent/Tracing.grpc.pb.h>

#include "app/include/api/call_data.h"
#include "trace_segment_server_event_data.h"
#include "app/include/sink/channel/grpc_channel.h"

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
using grpc::ServerBuilder;
using namespace skywalking::v3;

class TraceSegmentServerCallData : public Api::CallData {
public:
    TraceSegmentServerCallData(grpc::ServerCompletionQueue* cq, TraceSegmentReportService::AsyncService & service_,
                               std::unique_ptr<Core::Component::Pipeline>& pipeline)
        : cq_(cq), service(service_), pipeline_(pipeline) {
        Proceed();
    }

    void onCreate() final {
        service.Requestcollect(&ctx_, &reader, cq_, cq_, this);
    }

    void onProcess() final;

    void onFinish() final {
    }

    ~TraceSegmentServerCallData() {
    }

private:
    void handleData();

    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::ServerCompletionQueue* cq_;

    // What we get from the client.
    ::grpc::ServerAsyncReader<::skywalking::v3::Commands, ::skywalking::v3::SegmentObject> reader{&ctx_};
    // What we send back to the client.
    ::skywalking::v3::Commands reply_;

    SegmentObject object;
    TraceSegmentReportService::AsyncService & service;
    std::unique_ptr<Core::Component::Pipeline>& pipeline_;
    int times = 0;
};
}
}
}
}