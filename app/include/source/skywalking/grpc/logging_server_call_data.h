/**
******************************************************************************
* @file           : logging_server.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/2/27
******************************************************************************
*/
#pragma once

#include <grpcpp/completion_queue.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/interceptor.h>
#include <memory>
#include "app/include/api/call_data.h"
#include "common/Command.pb.h"
#include "logging/Logging.pb.h"
#include "skywalking-data-collect-protocol/logging/Logging.grpc.pb.h"
namespace Core { namespace Component { class Pipeline; } }

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
using namespace skywalking::v3;
class LoggingServerCallData : public Api::CallData {
public:
    LoggingServerCallData(grpc::ServerCompletionQueue* cq, LogReportService::AsyncService& service_,
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

private:
    void handleData();
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::ServerCompletionQueue *cq_;

    LogReportService::AsyncService &service;

    ::grpc::ServerAsyncReader<::skywalking::v3::Commands, ::skywalking::v3::LogData> reader{&ctx_};

    ::grpc::ServerAsyncResponseWriter<::skywalking::v3::Commands> response{&ctx_};

    std::unique_ptr<Core::Component::Pipeline> &pipeline_;

    LogData request;

    ::skywalking::v3::Commands reply_;

    LogData object;

    long times = 0;
};
}
}
}
}
