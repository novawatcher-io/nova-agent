set(NODE_PROTO_PATH "${PROJECT_SOURCE_DIR}")
set(NODE_GENERATED_PROTOBUF_PATH "${CMAKE_BINARY_DIR}/generated/")

add_library(nova_agent_config_proto OBJECT nova_agent_config.proto)

target_link_libraries(nova_agent_config_proto PUBLIC protobuf::libprotobuf)

set(PROTO_BINARY_DIR "${NODE_GENERATED_PROTOBUF_PATH}")
set(PROTO_IMPORT_DIRS "${NODE_PROTO_PATH}")

target_include_directories(nova_agent_config_proto PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

protobuf_generate(
        TARGET
        nova_agent_config_proto
        OUT_VAR
        PROTO_GENERATED_FILES
        IMPORT_DIRS
        ${PROTO_IMPORT_DIRS}
        PROTOC_OUT_DIR
        "${PROTO_BINARY_DIR}"
        PROTOC_OPTIONS
        "--experimental_allow_proto3_optional")

foreach (file ${PROTO_GENERATED_FILES})
    set_source_files_properties(${file} PROPERTIES SKIP_LINTING ON)
endforeach ()
