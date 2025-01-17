/**
******************************************************************************
* @file           : grpc_channel.cc
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/3/3
******************************************************************************
*/
//
// Created by zhanglei on 2024/3/3.
//
#include "app/include/api/call_data.h"
#include "app/include/sink/channel/batch.h"
#include "app/include/sink/channel/grpc_channel.h"

using namespace App::Sink::Channel;

void GrpcChannel::run() {
    void* tag; // uniquely identifies a request.
    bool ok;
    while (true) {
        // Block waiting to read the next event from the completion queue. The
        // event is uniquely identified by its tag, which in this case is the
        // memory address of a CallData instance.
        // The return value of Next should always be checked. This return value
        // tells us whether there is any kind of event or cq_ is shutting down.
        //        GPR_ASSERT(cq_->AsyncNext(&tag, &ok,  gpr_time_from_seconds(5,GPR_TIMESPAN)));
        GPR_ASSERT(cq_->Next(&tag, &ok));
        flush();

        if (ok) {
            static_cast<Api::CallData*>(tag)->Proceed();
        }
    }
}

void GrpcChannel::flush() {
    for (auto& iter : pipelines) {
        iter->flush();
    }
}
