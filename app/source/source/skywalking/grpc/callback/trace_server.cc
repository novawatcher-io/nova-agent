#include "app/include/source/skywalking/grpc/callback/trace_server.h"

#include "app/include/source/skywalking/grpc/trace_segment_server_event_data.h"
#include <grpcpp/support/status.h>
#include <grpcpp/support/string_ref.h>
#include <spdlog/spdlog.h>

namespace App::Source::SkyWalking::Grpc::Callback {
using namespace ::skywalking::v3;
::grpc::Status TraceServer::collect(::grpc::ServerContext* context,
                                    ::grpc::ServerReader<::skywalking::v3::SegmentObject>* reader,
                                    ::skywalking::v3::Commands* /*command*/) {
    const auto& metadata = context->client_metadata();
    grpc::string_ref authentication;
    auto it = metadata.find("authentication");
    if (it != metadata.end() && !it->second.empty()) {
        // SPDLOG_INFO("trace server token: {}", it->second.data());
        authentication = it->second;
    }
    skywalking::v3::SegmentObject object;
    while (reader->Read(&object)) {
        if (!object.IsInitialized()) {
            continue;
        }
        std::cout << object.DebugString() << std::endl;

        std::unique_ptr<SegmentObject> save_data = std::make_unique<SegmentObject>();
        save_data->CopyFrom(object);
        auto segmentData = std::make_unique<Core::Component::AnyData<SegmentObject>>(std::move(save_data));
        auto data = std::make_unique<TraceSegmentServerEventData>(std::move(segmentData));
        if (!authentication.empty()) {
            data->addMeta("authentication", std::string(authentication.data(), authentication.size()));
        }
        tracePipeline->push(std::move(data));
    }
    return ::grpc::Status::OK;
}
} // namespace App::Source::SkyWalking::Grpc::Callback
