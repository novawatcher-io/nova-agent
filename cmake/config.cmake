set(INSTALL_DIR "${PROJECT_SOURCE_DIR}")
set(PROTO_PATH "${INSTALL_DIR}")
set(PROTOBUF_RUN_PROTOC_COMMAND "\"${PROTOBUF_PROTOC_EXECUTABLE}\"")

set(CONFIG_COMMON_PROTO
        "${PROTO_PATH}/proto/common/common.proto")

set(CONFIG_RUNNER_CONFIG_PROTO
        "${PROTO_PATH}/proto/runner_config.proto")

set(CONFIG_PROTO_SOURCE_SKYWALKING_PROTO
        "${PROTO_PATH}/proto/source/skywalking/skywalking.proto")
set(CONFIG_PROTO_SOURCE_SKYWALKING_GRPC_CONFIG_PROTO
        "${PROTO_PATH}/proto/source/skywalking/grpc/config.proto")

set(CONFIG_PROTO_INTERCEPT_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PROTO
        "${PROTO_PATH}/proto/intercept/opentelemetry/log/skywalking/config.proto")
set(CONFIG_PROTO_INTERCEPT_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PROTO
        "${PROTO_PATH}/proto/intercept/opentelemetry/metric/skywalking/config.proto")
set(CONFIG_PROTO_INTERCEPT_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PROTO
        "${PROTO_PATH}/proto/intercept/opentelemetry/trace/skywalking/config.proto")
set(CONFIG_PROTO_INTERCEPT_OPENTELEMETRY_OPENTELEMETRY_PROTO
        "${PROTO_PATH}/proto/intercept/opentelemetry/opentelemetry.proto")

set(CONFIG_PROTO_SINK_OPENTELEMETRY_LOG_GRPC_CONFIG_PROTO
        "${PROTO_PATH}/proto/sink/opentelemetry/log/grpc/config.proto")
set(CONFIG_PROTO_SINK_OPENTELEMETRY_METRIC_GRPC_CONFIG_PROTO
        "${PROTO_PATH}/proto/sink/opentelemetry/metric/grpc/config.proto")
set(CONFIG_PROTO_SINK_OPENTELEMETRY_TRACE_GRPC_CONFIG_PROTO
        "${PROTO_PATH}/proto/sink/opentelemetry/trace/grpc/config.proto")
set(CONFIG_PROTO_SINK_OPENTELEMETRY_OPENTELEMETRY_PROTO
        "${PROTO_PATH}/proto/sink/opentelemetry/opentelemetry.proto")



set(GENERATED_CONFIG_PROTOBUF_PATH
        "${CMAKE_BINARY_DIR}/generated/config")
message("GENERATED_CONFIG_PROTOBUF_PATH:${GENERATED_CONFIG_PROTOBUF_PATH}")
file(MAKE_DIRECTORY "${GENERATED_CONFIG_PROTOBUF_PATH}")



set(CONFIG_COMMON_PROTO_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/common/common.pb.cc")
set(CONFIG_COMMON_PROTO_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/common/common.pb.h")


set(SOURCE_SKYWALKING_SKYWALKING_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/source/skywalking/skywalking.pb.cc")
set(SOURCE_SKYWALKING_SKYWALKING_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/source/skywalking/skywalking.pb.h")
set(SOURCE_SKYWALKING_SKYWALKING_GRPC_CONFIG_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/source/skywalking/grpc/config.pb.cc")
set(SOURCE_SKYWALKING_SKYWALKING_GRPC_CONFIG_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/source/skywalking/grpc/config.pb.h")

set(INTERCEPT_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/intercept/opentelemetry/log/skywalking/config.pb.cc")
set(INTERCEPT_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/intercept/opentelemetry/log/skywalking/config.pb.h")
set(INTERCEPT_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/intercept/opentelemetry/metric/skywalking/config.pb.cc")
set(INTERCEPT_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/intercept/opentelemetry/metric/skywalking/config.pb.h")
set(INTERCEPT_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/intercept/opentelemetry/trace/skywalking/config.pb.cc")
set(INTERCEPT_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/intercept/opentelemetry/trace/skywalking/config.pb.h")
set(INTERCEPT_OPENTELEMETRY_OPENTELEMETRY_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/intercept/opentelemetry/opentelemetry.pb.cc")
set(INTERCEPT_OPENTELEMETRY_OPENTELEMETRY_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/intercept/opentelemetry/opentelemetry.pb.h")


set(SINK_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/sink/opentelemetry/log/grpc/config.pb.cc")
set(SINK_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/sink/opentelemetry/log/grpc/config.pb.h")
set(SINK_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/sink/opentelemetry/metric/grpc/config.pb.cc")
set(SINK_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/sink/opentelemetry/metric/grpc/config.pb.h")
set(SINK_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/sink/opentelemetry/trace/grpc/config.pb.cc")
set(SINK_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/sink/opentelemetry/trace/grpc/config.pb.h")
set(SINK_OPENTELEMETRY_OPENTELEMETRY_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/sink/opentelemetry/opentelemetry.pb.cc")
set(SINK_OPENTELEMETRY_OPENTELEMETRY_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/sink/opentelemetry/opentelemetry.pb.h")

set(RUNNER_CONFIG_PB_CPP_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/runner_config.pb.cc")
set(RUNNER_CONFIG_PB_H_FILE
        "${GENERATED_CONFIG_PROTOBUF_PATH}/proto/runner_config.pb.h")

set(AGENT_CONFIG_PROTOBUF_GENERATED_FILES
        ${RUNNER_CONFIG_PB_CPP_FILE}
        ${RUNNER_CONFIG_PB_H_FILE}

        ${CONFIG_COMMON_PROTO_PB_CPP_FILE}
        ${CONFIG_COMMON_PROTO_PB_H_FILE}

        ${SOURCE_SKYWALKING_SKYWALKING_PB_CPP_FILE}
        ${SOURCE_SKYWALKING_SKYWALKING_PB_H_FILE}
        ${SOURCE_SKYWALKING_SKYWALKING_GRPC_CONFIG_PB_CPP_FILE}
        ${SOURCE_SKYWALKING_SKYWALKING_GRPC_CONFIG_PB_H_FILE}

        ${INTERCEPT_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PB_CPP_FILE}
        ${INTERCEPT_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PB_H_FILE}
        ${INTERCEPT_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PB_CPP_FILE}
        ${INTERCEPT_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PB_H_FILE}
        ${INTERCEPT_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PB_CPP_FILE}
        ${INTERCEPT_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PB_H_FILE}
        ${INTERCEPT_OPENTELEMETRY_OPENTELEMETRY_PB_CPP_FILE}
        ${INTERCEPT_OPENTELEMETRY_OPENTELEMETRY_PB_H_FILE}

        ${SINK_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PB_CPP_FILE}
        ${SINK_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PB_H_FILE}
        ${SINK_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PB_CPP_FILE}
        ${SINK_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PB_H_FILE}
        ${SINK_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PB_CPP_FILE}
        ${SINK_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PB_H_FILE}
        ${SINK_OPENTELEMETRY_OPENTELEMETRY_PB_CPP_FILE}
        ${SINK_OPENTELEMETRY_OPENTELEMETRY_PB_H_FILE}
        )

foreach(IMPORT_DIR ${PROTOBUF_IMPORT_DIRS})
    list(APPEND PROTOBUF_INCLUDE_FLAGS "-I${IMPORT_DIR}")
endforeach()


set(PROTOBUF_COMMON_FLAGS "--proto_path=${PROTO_PATH}"
        "--cpp_out=${GENERATED_CONFIG_PROTOBUF_PATH}")

# --experimental_allow_proto3_optional is available from 3.13 and be stable and
# enabled by default from 3.16
if(Protobuf_VERSION AND Protobuf_VERSION VERSION_LESS "3.16")
    list(APPEND PROTOBUF_COMMON_FLAGS "--experimental_allow_proto3_optional")
elseif(PROTOBUF_VERSION AND PROTOBUF_VERSION VERSION_LESS "3.16")
    list(APPEND PROTOBUF_COMMON_FLAGS "--experimental_allow_proto3_optional")
endif()


list(
        APPEND
        AGENT_CONFIG_PROTOBUF_GENERATED_FILES

)


if(NOT EXISTS ${CONFIG_COMMON_PROTO_PB_CPP_FILE})
    add_custom_command(
            OUTPUT ${AGENT_CONFIG_PROTOBUF_GENERATED_FILES}
            COMMAND
            ${_trace_agent_PROTOBUF_PROTOC_EXECUTABLE} ${PROTOBUF_COMMON_FLAGS}
            ${PROTOBUF_INCLUDE_FLAGS}

            ${CONFIG_RUNNER_CONFIG_PROTO}

            ${CONFIG_COMMON_PROTO}
            ${CONFIG_PROTO_SOURCE_SKYWALKING_PROTO}
            ${CONFIG_PROTO_SOURCE_SKYWALKING_GRPC_CONFIG_PROTO}

            ${CONFIG_PROTO_INTERCEPT_OPENTELEMETRY_LOG_SKYWALKING_CONFIG_PROTO}
            ${CONFIG_PROTO_INTERCEPT_OPENTELEMETRY_METRIC_SKYWALKING_CONFIG_PROTO}
            ${CONFIG_PROTO_INTERCEPT_OPENTELEMETRY_TRACE_SKYWALKING_CONFIG_PROTO}
            ${CONFIG_PROTO_INTERCEPT_OPENTELEMETRY_OPENTELEMETRY_PROTO}

            ${CONFIG_PROTO_SINK_OPENTELEMETRY_LOG_GRPC_CONFIG_PROTO}
            ${CONFIG_PROTO_SINK_OPENTELEMETRY_METRIC_GRPC_CONFIG_PROTO}
            ${CONFIG_PROTO_SINK_OPENTELEMETRY_TRACE_GRPC_CONFIG_PROTO}
            ${CONFIG_PROTO_SINK_OPENTELEMETRY_OPENTELEMETRY_PROTO}
            COMMENT "[Run]: ${PROTOBUF_RUN_PROTOC_COMMAND}")
endif()

include_directories("${GENERATED_CONFIG_PROTOBUF_PATH}")
add_library(
        agent_config_proto
        ${AGENT_CONFIG_PROTOBUF_GENERATED_FILES}
)

foreach(file ${AGENT_CONFIG_PROTOBUF_GENERATED_FILES})
    set_source_files_properties(${file} PROPERTIES SKIP_LINTING ON)
endforeach()
