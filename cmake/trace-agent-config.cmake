set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/proto")
set(PROTO_PATH "${INSTALL_DIR}")
set(PROTOBUF_RUN_PROTOC_COMMAND "\"${PROTOBUF_PROTOC_EXECUTABLE}\"")

set(TRACE_AGENT_COMMON_PROTO
        "${PROTO_PATH}/common/common.proto")
set(TRACE_AGENT_SOURCE_SKYWALKING_GRPC_CONFIG_PROTO
        "${PROTO_PATH}/source/skywalking/grpc/config.proto")
set(TRACE_AGENT_SINK_OPENTELEMETRY_LOG_GRPC_CONFIG_PROTO
        "${PROTO_PATH}/sink/opentelemetry/log/grpc/config.proto")

set(TRACE_AGENT_SINK_OPENTELEMETRY_METRIC_GRPC_CONFIG_PROTO
        "${PROTO_PATH}/sink/opentelemetry/metric/grpc/config.proto")
set(TRACE_AGENT_SINK_OPENTELEMETRY_TRACE_GRPC_CONFIG_PROTO
        "${PROTO_PATH}/sink/opentelemetry/trace/grpc/config.proto")


set(TRACE_AGENT_PROTOBUF_GENERATED_FILES
        ${TRACE_AGENT_COMMON_PROTO}
        ${TRACE_AGENT_SOURCE_SKYWALKING_GRPC_CONFIG_PROTO}
        ${TRACE_AGENT_SINK_OPENTELEMETRY_LOG_GRPC_CONFIG_PROTO}
        ${TRACE_AGENT_SINK_OPENTELEMETRY_METRIC_GRPC_CONFIG_PROTO}
        ${TRACE_AGENT_SINK_OPENTELEMETRY_TRACE_GRPC_CONFIG_PROTO})

set(BROWSER_BROWSERPERF_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/browser/BrowserPerf.pb.cc")
set(BROWSER_BROWSERPERF_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/browser/BrowserPerf.pb.h")
set(BROWSER_BROWSERPERFCOMPAT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/browser/BrowserPerfCompat.pb.cc")
set(BROWSER_BROWSERPERFCOMPAT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/browser/BrowserPerfCompat.pb.h")

foreach(IMPORT_DIR ${PROTOBUF_IMPORT_DIRS})
    list(APPEND PROTOBUF_INCLUDE_FLAGS "-I${IMPORT_DIR}")
endforeach()

include_directories("${CMAKE_BINARY_DIR}/generated/third_party")

message(STATUS "${COMMON_PROTO} ${RESOURCE_PROTO} ${TRACE_PROTO}
        ${LOGS_PROTO} ${METRICS_PROTO} ${TRACE_SERVICE_PROTO} ${LOGS_SERVICE_PROTO}
        ${METRICS_SERVICE_PROTO}")

if(CMAKE_CROSSCOMPILING)
    find_program(gRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    if(TARGET gRPC::grpc_cpp_plugin)
        project_build_tools_get_imported_location(gRPC_CPP_PLUGIN_EXECUTABLE
                gRPC::grpc_cpp_plugin)
    else()
        find_program(gRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    endif()
endif()
message(STATUS "gRPC_CPP_PLUGIN_EXECUTABLE=${gRPC_CPP_PLUGIN_EXECUTABLE}")

set(PROTOBUF_COMMON_FLAGS "--proto_path=${PROTO_PATH}"
        "--cpp_out=${GENERATED_PROTOBUF_PATH}")

# --experimental_allow_proto3_optional is available from 3.13 and be stable and
# enabled by default from 3.16
if(Protobuf_VERSION AND Protobuf_VERSION VERSION_LESS "3.16")
    list(APPEND PROTOBUF_COMMON_FLAGS "--experimental_allow_proto3_optional")
elseif(PROTOBUF_VERSION AND PROTOBUF_VERSION VERSION_LESS "3.16")
    list(APPEND PROTOBUF_COMMON_FLAGS "--experimental_allow_proto3_optional")
endif()

list(APPEND PROTOBUF_COMMON_FLAGS
        "--grpc_out=generate_mock_code=true:${GENERATED_PROTOBUF_PATH}"
        --plugin=protoc-gen-grpc="${gRPC_CPP_PLUGIN_EXECUTABLE}")

list(
        APPEND
        SKYWALKING_PROTOBUF_GENERATED_FILES
        ${TRACING_GRPC_PB_H_FILE}
        ${TRACING_GRPC_PB_CPP_FILE}
        ${LOGGING_LOGGING_GRPC_PB_H_FILE}
        ${LOGGING_LOGGING_GRPC_PB_CPP_FILE}
        ${JVMMETRIC_GRPC_PB_H_FILE}
        ${JVMMETRIC_GRPC_PB_CPP_FILE}
        ${CONFIGURATION_DISCOVERY_SERVICE_GRPC_PB_CPP_FILE}
        ${CONFIGURATION_DISCOVERY_SERVICE_GRPC_PB_H_FILE}
        ${METER_GRPC_PB_CPP_FILE}
        ${METER_GRPC_PB_H_FILE}
        ${MANAGEMENT_MANAGEMENT_GRPC_PB_CPP_FILE}
        ${MANAGEMENT_MANAGEMENT_GRPC_PB_H_FILE}
)


if(NOT EXISTS ${TRACING_GRPC_PB_CPP_FILE})
    add_custom_command(
            OUTPUT ${SKYWALKING_PROTOBUF_GENERATED_FILES}
            COMMAND
            ${_trace_agent_PROTOBUF_PROTOC_EXECUTABLE} ${PROTOBUF_COMMON_FLAGS}
            ${PROTOBUF_INCLUDE_FLAGS} ${BROWSERPERF_PROTO} ${BROWSERPERFCOMPAT_PROTO} ${COMMON_PROTO}
            ${COMMAND_PROTO} ${EBPF_PROFILING_CONTINUOUS_PROTO} ${EBPF_PROFILING_PROCESS_PROTO} ${EBPF_PROFILING_PROFILE_PROTO}
            ${EBPF_ACCESSLOG_PROTO} ${EBPF_EVENT_EVENT_PROTO} ${LANGUAGE_AGENT_CLRMETRIC_PROTO} ${LANGUAGE_AGENT_CLRMETRICCOMPAT_PROTO}
            ${LANGUAGE_AGENT_CONFIGURATIONDISCOVERYSERVICE_PROTO}
            ${LANGUAGE_AGENT_JVMMETRIC_PROTO} ${LANGUAGE_AGENT_JVMMETRICCOMPAT_PROTO} ${LANGUAGE_AGENT_METER_PROTO}
            ${LANGUAGE_AGENT_METERCOMPAT_PROTO} ${LANGUAGE_AGENT_TRACING_PROTO}
            ${LANGUAGE_AGENT_TRACINGCOMPAT_PROTO} ${LOGGING_LOGGING_PROTO} ${MANAGEMENT_MANAGEMENT_PROTO}
            ${MANAGEMENT_MANAGEMENTCOMPAT_PROTO} ${PROFILE_PROFILE_PROTO}
            ${PROFILE_PROFILECOMPAT_PROTO} ${SERVICE_MESH_PROBE_SERVICE_MESH_PROTO}
            COMMENT "[Run]: ${PROTOBUF_RUN_PROTOC_COMMAND}")
endif()

include_directories("${GENERATED_PROTOBUF_PATH}")
message(STATUS "BROWSERPERF_PROTO:${SKYWALKING_PROTOBUF_GENERATED_FILES}")

foreach(file ${SKYWALKING_PROTOBUF_GENERATED_FILES})
        set_source_files_properties(${file} PROPERTIES SKIP_LINTING ON)
endforeach()
