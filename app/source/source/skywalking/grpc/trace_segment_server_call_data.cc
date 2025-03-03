#include "app/include/source/skywalking/grpc/trace_segment_server_call_data.h"
#include <opentelemetry-proto/opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>
#include "os/unix_util.h"

using namespace App::Source::SkyWalking::Grpc;

void TraceSegmentServerCallData::onProcess() {
    // Spawn a new CallData instance to serve new clients while we process
    // the one for this CallData. The instance will deallocate itself as
    // part of its FINISH state.
    if (times == 0) {
        new TraceSegmentServerCallData(cq_, service, pipeline_);
        reader.Read(&object, this);
//        std::cout << "debug:" << object.DebugString() << std::endl;
        ++times;
        reader.Finish(reply_, grpc::Status::OK, this);
        return;

    }
    if (++times > 1000) {
        reader.Finish(reply_, grpc::Status::OK, this);
        finish();
        return;
    } else {
        handleData();
        reader.Read(&object, this);
    }
}

void TraceSegmentServerCallData::handleData() {
    if (!object.IsInitialized()) {
        return;
    }
    std::unique_ptr<SegmentObject> save_data = std::make_unique<SegmentObject>();
    save_data->CopyFrom(object);
    auto segmentData = std::make_unique<Core::Component::AnyData<SegmentObject>>(std::move(save_data));
    auto data = std::make_unique<TraceSegmentServerEventData>(std::move(segmentData));
    pipeline_->push(std::move(data));
    return;
}
