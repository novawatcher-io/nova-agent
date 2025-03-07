#pragma once

#include <skywalking-data-collect-protocol/language-agent/Meter.grpc.pb.h>

namespace App {
namespace Source {
namespace SkyWalking {
namespace Grpc {
namespace Callback {

class MeterReportServer :public skywalking::v3::MeterReportService::Service {
public:
    ::grpc::Status collect(::grpc::ServerContext*, ::grpc::ServerReader< ::skywalking::v3::MeterData>*, ::skywalking::v3::Commands*) {
        return ::grpc::Status::OK;
    }
    // Reporting meter data in bulk mode as MeterDataCollection.
    // By using this, each one in the stream would be treated as a complete input for MAL engine,
    // comparing to `collect (stream MeterData)`, which is using one stream as an input data set.
    ::grpc::Status collectBatch(::grpc::ServerContext*, ::grpc::ServerReader< ::skywalking::v3::MeterDataCollection>* reader, ::skywalking::v3::Commands*) {
        ::skywalking::v3::MeterDataCollection collection;
         while (reader->Read(&collection)) {

         }
        return ::grpc::Status::OK;
    }
};

}
}
}
}
}