set(CRI_API_PATH "${PROJECT_SOURCE_DIR}/proto/cri-api")
set(NODE_GENERATED_PROTOBUF_PATH "${CMAKE_BINARY_DIR}/generated/cri-api")
file(MAKE_DIRECTORY ${NODE_GENERATED_PROTOBUF_PATH})

add_library(cri_api_proto OBJECT ${CRI_API_PATH}/api.proto
        ${CRI_API_PATH}/gogo.proto)

target_link_libraries(cri_api_proto PUBLIC protobuf::libprotobuf)

set(PROTO_BINARY_DIR "${NODE_GENERATED_PROTOBUF_PATH}")
set(PROTO_IMPORT_DIRS "${CRI_API_PATH}")

target_include_directories(cri_api_proto PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

protobuf_generate(
        TARGET
        cri_api_proto
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

protobuf_generate(
    TARGET cri_api_proto
    OUT_VAR GRPC_GENERATED_FILES
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    PLUGIN_OPTIONS "generate_mock_code=true"
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")

target_link_libraries(cri_api_proto PUBLIC gRPC::grpc++ gRPC::grpc gRPC::grpc++_reflection gRPC::grpc++_unsecure)

foreach (file ${GRPC_GENERATED_FILES})
    set_source_files_properties(${file} PROPERTIES SKIP_LINTING ON)
endforeach ()
