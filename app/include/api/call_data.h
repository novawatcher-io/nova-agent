/**
******************************************************************************
* @file           : call_data.h
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

#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <pure.h>
#include <os/unix_util.h>

namespace App {
namespace Api {
enum class CallStatus {
    CREATE,
    PROCESS,
    FINISH
};

class CallData {
public:
    CallData() : status_(CallStatus::CREATE) {
    }

    void Proceed() {
        if (status_ == CallStatus::CREATE) {
            // Make this instance progress to the PROCESS state.
            status_ = CallStatus::PROCESS;

            onCreate();
        } else if (status_ == CallStatus::PROCESS) {
            onProcess();
        } else {
            GPR_ASSERT(status_ == CallStatus::FINISH);
            onFinish();
            // Once in the FINISH state, deallocate ourselves (CallData).
            delete this;
        }
    }

    virtual void onCreate() PURE;

    virtual void onProcess() PURE;

    virtual void onFinish() PURE;

    virtual ~CallData() = default;

protected:
    void finish() {
        GPR_ASSERT(status_ == CallStatus::PROCESS);
        status_ = CallStatus::FINISH;
    }
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    grpc::ServerContext ctx_;

private:
    // Let's implement a tiny state machine with the following states.
    CallStatus status_;  // The current serving state.
};
}
}

