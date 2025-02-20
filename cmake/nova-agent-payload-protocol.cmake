set(NODE_PROTO_PATH "${PROJECT_SOURCE_DIR}/third_party/nova-agent-payload")
set(NODE_GENERATED_PROTOBUF_PATH "${CMAKE_BINARY_DIR}/generated/third_party/nova_agent_payload")
file(MAKE_DIRECTORY ${NODE_GENERATED_PROTOBUF_PATH})

add_library(node_data_collect_protocol_proto OBJECT "${NODE_PROTO_PATH}/node/v1/info.proto" "${NODE_PROTO_PATH}/trace/v1/topology.proto")

target_link_libraries(node_data_collect_protocol_proto PUBLIC protobuf::libprotobuf gRPC::grpc++)

set(PROTO_BINARY_DIR "${NODE_GENERATED_PROTOBUF_PATH}")
set(PROTO_IMPORT_DIRS "${NODE_PROTO_PATH}")

target_include_directories(node_data_collect_protocol_proto PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

protobuf_generate(
    TARGET
    node_data_collect_protocol_proto
    OUT_VAR
    PROTO_GENERATED_FILES
    IMPORT_DIRS
    ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR
    "${PROTO_BINARY_DIR}"
    PROTOC_OPTIONS
    "--experimental_allow_proto3_optional")

foreach(file ${PROTO_GENERATED_FILES})
    set_source_files_properties(${file} PROPERTIES SKIP_LINTING ON)
endforeach()

protobuf_generate(
    TARGET
    node_data_collect_protocol_proto
    OUT_VAR
    GRPC_GENERATED_FILES
    LANGUAGE
    grpc
    GENERATE_EXTENSIONS
    .grpc.pb.h
    .grpc.pb.cc
    PLUGIN
    "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    PLUGIN_OPTIONS
    "generate_mock_code=true"
    IMPORT_DIRS
    ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR
    "${PROTO_BINARY_DIR}")

foreach(file ${GRPC_GENERATED_FILES})
    set_source_files_properties(${file} PROPERTIES SKIP_LINTING ON)
endforeach()
