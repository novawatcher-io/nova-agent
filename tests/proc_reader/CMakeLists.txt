add_executable(test_proc_reader proc_reader_test.cc)
target_link_libraries(test_proc_reader PRIVATE GTest::gtest GTest::gtest_main GTest::gmock process-collector)
add_test(NAME test_proc_reader COMMAND test_proc_reader)

add_executable(oltp_reporter oltp_reporter.cc)
target_link_libraries(oltp_reporter opentelemetry-cpp::common
        opentelemetry-cpp::trace
        opentelemetry-cpp::resources
        opentelemetry-cpp::sdk
        opentelemetry-cpp::otlp_grpc_exporter
        opentelemetry-cpp::otlp_grpc_metrics_exporter
        opentelemetry-cpp::in_memory_span_exporter
        opentelemetry-cpp::metrics)

add_executable(gpu_reader gpu.cc)
target_link_libraries(gpu_reader opentelemetry-cpp::common
        opentelemetry-cpp::trace
        opentelemetry-cpp::resources
        opentelemetry-cpp::sdk
        opentelemetry-cpp::otlp_grpc_exporter
        opentelemetry-cpp::otlp_grpc_metrics_exporter
        opentelemetry-cpp::in_memory_span_exporter
        opentelemetry-cpp::metrics)