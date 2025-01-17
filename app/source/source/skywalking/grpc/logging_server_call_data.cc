#include "app/include/source/skywalking/grpc/logging_server_call_data.h"
#include <grpcpp/support/status.h>
#include <iostream>
#include <string>
#include <utility>
#include "app/include/source/skywalking/grpc/logging_server_event_data.h"
#include "component/api.h"

using namespace App::Source::SkyWalking::Grpc;

void LoggingServerCallData::onProcess() {
    if (times == 0) {
        new LoggingServerCallData(cq_, service, pipeline_);
    }

    if (++times > 1000) {
        reader.Finish(reply_, grpc::Status::OK, this);
        finish();
        return;
    } else {
        if (times > 1) {
            handleData();
        }
        std::cout << object.DebugString() << std::endl;
        reader.Read(&object, this);
    }
}

void LoggingServerCallData::handleData() {
    std::unique_ptr<::skywalking::v3::LogData> collection = std::make_unique<::skywalking::v3::LogData>();
    collection->CopyFrom(request);
    auto segmentData = std::make_unique<Core::Component::AnyData<LogData>>(std::move(collection));
    auto data = std::make_unique<LoggingServerEventData>(
        std::make_unique<Core::Component::AnyData<::skywalking::v3::LogData>>(std::move(collection)));
    pipeline_->push(std::move(data));
    return;
}
