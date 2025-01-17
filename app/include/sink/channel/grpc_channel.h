/**
******************************************************************************
* @file           : grpc_channel.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/3/3
******************************************************************************
*/
//
// Created by zhanglei on 2024/3/3.
//
#pragma once

#include <app/include/intercept/chain.h>
#include <component/api.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <list>
#include <unordered_map>

#include "batch.h"

namespace App {
namespace Sink {
namespace Channel {
using namespace Core::Component;
class GrpcChannel : public Core::Component::Queue {
public:
    explicit GrpcChannel(std::unique_ptr<grpc::CompletionQueue>& cq) : cq_(cq) {
    }
    void addPipelines(std::unique_ptr<Core::Component::Pipeline> pipeline) {
        pipelines.push_back(std::move(pipeline));
    }

    void run();

    void stop() {
        cq_->Shutdown();
    }

private:
    void flush();
    std::unique_ptr<grpc::CompletionQueue>& cq_;
    std::list<std::unique_ptr<Core::Component::Pipeline>> pipelines;
};
}
}
}