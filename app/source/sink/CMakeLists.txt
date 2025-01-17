add_library(
    sink
    channel/grpc_channel.cc
    deep_observe/node/client.cc
    deep_observe/node/sink.cc
    opentelemetry/metric/grpc/sink.cc
    opentelemetry/trace/grpc/sink.cc
    pipeline/pipeline.cc)
