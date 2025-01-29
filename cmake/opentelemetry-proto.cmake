set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/third_party/opentelemetry-proto")
set(PROTO_PATH "${INSTALL_DIR}")
set(PROTOBUF_RUN_PROTOC_COMMAND "\"${PROTOBUF_PROTOC_EXECUTABLE}\"")

set(COMMON_PROTO "${PROTO_PATH}/opentelemetry/proto/common/v1/common.proto")
set(RESOURCE_PROTO
        "${PROTO_PATH}/opentelemetry/proto/resource/v1/resource.proto")
set(TRACE_PROTO "${PROTO_PATH}/opentelemetry/proto/trace/v1/trace.proto")
set(LOGS_PROTO "${PROTO_PATH}/opentelemetry/proto/logs/v1/logs.proto")
set(METRICS_PROTO "${PROTO_PATH}/opentelemetry/proto/metrics/v1/metrics.proto")

set(TRACE_SERVICE_PROTO
        "${PROTO_PATH}/opentelemetry/proto/collector/trace/v1/trace_service.proto")
set(LOGS_SERVICE_PROTO
        "${PROTO_PATH}/opentelemetry/proto/collector/logs/v1/logs_service.proto")
set(METRICS_SERVICE_PROTO
        "${PROTO_PATH}/opentelemetry/proto/collector/metrics/v1/metrics_service.proto"
        )

set(GENERATED_PROTOBUF_PATH
        "${CMAKE_BINARY_DIR}/generated/third_party/opentelemetry-proto")

message(STATUS "${GENERATED_PROTOBUF_PATH}")

file(MAKE_DIRECTORY "${GENERATED_PROTOBUF_PATH}")

set(COMMON_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/common/v1/common.pb.cc")
set(COMMON_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/common/v1/common.pb.h")
set(RESOURCE_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/resource/v1/resource.pb.cc")
set(RESOURCE_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/resource/v1/resource.pb.h")
set(TRACE_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/trace/v1/trace.pb.cc")
set(TRACE_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/trace/v1/trace.pb.h")
set(LOGS_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/logs/v1/logs.pb.cc")
set(LOGS_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/logs/v1/logs.pb.h")
set(METRICS_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/metrics/v1/metrics.pb.cc")
set(METRICS_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/metrics/v1/metrics.pb.h")

set(TRACE_SERVICE_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/trace/v1/trace_service.pb.cc"
        )
set(TRACE_SERVICE_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/trace/v1/trace_service.pb.h"
        )

set(LOGS_SERVICE_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
        )
set(LOGS_SERVICE_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/logs/v1/logs_service.pb.cc"
        )
set(METRICS_SERVICE_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h"
        )

set(METRICS_SERVICE_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/metrics/v1/metrics_service.pb.cc"
        )
set(TRACE_SERVICE_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.cc"
        )
set(TRACE_SERVICE_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h"
        )
set(LOGS_SERVICE_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.cc"
        )
set(LOGS_SERVICE_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.h"
        )
set(METRICS_SERVICE_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.cc"
        )
set(METRICS_SERVICE_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h"
        )

set(PROTOBUF_GENERATED_FILES
        ${COMMON_PB_H_FILE}
        ${COMMON_PB_CPP_FILE}
        ${RESOURCE_PB_H_FILE}
        ${RESOURCE_PB_CPP_FILE}
        ${TRACE_PB_H_FILE}
        ${TRACE_PB_CPP_FILE}
        ${LOGS_PB_H_FILE}
        ${LOGS_PB_CPP_FILE}
        ${METRICS_PB_H_FILE}
        ${METRICS_PB_CPP_FILE}
        ${TRACE_SERVICE_PB_H_FILE}
        ${TRACE_SERVICE_PB_CPP_FILE}
        ${LOGS_SERVICE_PB_H_FILE}
        ${LOGS_SERVICE_PB_CPP_FILE}
        ${METRICS_SERVICE_PB_H_FILE}
        ${METRICS_SERVICE_PB_CPP_FILE})

foreach(IMPORT_DIR ${PROTOBUF_IMPORT_DIRS})
    list(APPEND PROTOBUF_INCLUDE_FLAGS "-I${IMPORT_DIR}")
endforeach()

message(STATUS "${COMMON_PROTO} ${RESOURCE_PROTO} ${TRACE_PROTO}
        ${LOGS_PROTO} ${METRICS_PROTO} ${TRACE_SERVICE_PROTO} ${LOGS_SERVICE_PROTO}
        ${METRICS_SERVICE_PROTO}")

if(nova_agent_GRPC_PROVIDER STREQUAL "package")
    find_program(gRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    if(TARGET gRPC::grpc_cpp_plugin)
        project_build_tools_get_imported_location(gRPC_CPP_PLUGIN_EXECUTABLE
                gRPC::grpc_cpp_plugin)
    elseif(TARGET grpc_cpp_plugin)
        message(STATUS "gRPC::grpc_cpp_plugin found")
        set(gRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
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
        PROTOBUF_GENERATED_FILES
        ${TRACE_SERVICE_GRPC_PB_H_FILE}
        ${TRACE_SERVICE_GRPC_PB_CPP_FILE}
        ${LOGS_SERVICE_GRPC_PB_H_FILE}
        ${LOGS_SERVICE_GRPC_PB_CPP_FILE}
        ${METRICS_SERVICE_GRPC_PB_H_FILE}
        ${METRICS_SERVICE_GRPC_PB_CPP_FILE}
)

if(NOT EXISTS ${TRACE_SERVICE_GRPC_PB_H_FILE})
    add_custom_command(
            OUTPUT ${PROTOBUF_GENERATED_FILES}
            COMMAND
            ${_nova_agent_PROTOBUF_PROTOC_EXECUTABLE} ${PROTOBUF_COMMON_FLAGS}
            ${PROTOBUF_INCLUDE_FLAGS} ${COMMON_PROTO} ${RESOURCE_PROTO} ${TRACE_PROTO}
            ${LOGS_PROTO} ${METRICS_PROTO} ${TRACE_SERVICE_PROTO} ${LOGS_SERVICE_PROTO}
            ${METRICS_SERVICE_PROTO}
            COMMENT "[Run]: ${PROTOBUF_RUN_PROTOC_COMMAND}")
endif()

include_directories("${GENERATED_PROTOBUF_PATH}")

add_library(
        nova_agent_opentelemetry_proto
        ${PROTOBUF_GENERATED_FILES}
)

foreach(file ${PROTOBUF_GENERATED_FILES})
        set_source_files_properties(${file} PROPERTIES SKIP_LINTING ON)
endforeach()
