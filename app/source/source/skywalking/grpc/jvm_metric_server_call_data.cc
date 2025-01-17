//
// Created by root on 2024/4/12.
//

#include "app/include/source/skywalking/grpc/jvm_metric_server_call_data.h"
#include "app/include/source/skywalking/grpc/jvm_metric_server_event_data.h"

#include <memory>

using namespace App::Source::SkyWalking::Grpc;

void JvmMetricServerCallData::onProcess() {
    new JvmMetricServerCallData(cq_, service, pipeline_);
    auto collection = std::make_unique<::skywalking::v3::JVMMetricCollection>();
    collection->CopyFrom(request);
    auto data = std::make_unique<JvmMetricServerEventData>(
        std::make_unique<Core::Component::AnyData<::skywalking::v3::JVMMetricCollection>>(std::move(collection)));
    pipeline_->push(std::move(data));
    response.Finish(reply_, grpc::Status::OK, this);
    finish();
}
