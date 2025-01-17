add_library(
    trace-agent-common
    grpc/grpc_client_options.cc opentelemetry/otlp_environment.cc opentelemetry/otlp_grpc_client.cc
    opentelemetry/otlp_metric_utils.cc opentelemetry/otlp_populate_attribute_utils.cc opentelemetry/recordable.cc)

target_link_libraries(trace-agent-common libcore)
