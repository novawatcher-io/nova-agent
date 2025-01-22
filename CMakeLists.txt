cmake_minimum_required(VERSION 3.8..3.28)

project(trace-agent C CXX)
set(APP_NAME trace-agent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# third party dependencies provider; module: build from source code package: find from system
set(trace_agent_ABSL_PROVIDER
    "package"
    CACHE STRING "Provider of Abseil-cpp library")
set_property(CACHE trace_agent_ABSL_PROVIDER PROPERTY STRINGS "module" "package")

set(trace_agent_PROTOBUF_PROVIDER
    "package"
    CACHE STRING "Provider of Protobuf library")
set_property(CACHE trace_agent_PROTOBUF_PROVIDER PROPERTY STRINGS "module" "package")

set(trace_agent_GRPC_PROVIDER
    "package"
    CACHE STRING "Provider of gRPC library")
set_property(CACHE trace_agent_GRPC_PROVIDER PROPERTY STRINGS "module" "package")

set(trace_agent_OPENTELEMETRY_CPP_PROVIDER
    "package"
    CACHE STRING "Provider of opentelemetry-cpp library")
set_property(CACHE trace_agent_OPENTELEMETRY_CPP_PROVIDER PROPERTY STRINGS "module" "package")

set(trace_agent_STDUUID_PROVIDER
    "package"
    CACHE STRING "Provider of stduuid library")
set_property(CACHE trace_agent_STDUUID_PROVIDER PROPERTY STRINGS "module" "package")

set(trace_agent_LIBCORE_PROVIDER
    "package"
    CACHE STRING "Provider of libcore library")
set_property(CACHE trace_agent_LIBCORE_PROVIDER PROPERTY STRINGS "module" "package")

set(trace_agent_CPU_FEATURES_PROVIDER
    "package"
    CACHE STRING "Provider of cpu_feature library")
set_property(CACHE trace_agent_LIBCORE_PROVIDER PROPERTY STRINGS "module" "package")

set(SANITIZER_TYPE
    "address"
    CACHE STRING "Choose the type of sanitizer: address or thread")
set_property(CACHE SANITIZER_TYPE PROPERTY STRINGS "address" "thread" "none")

option(ENABLE_IWYU "Enable Include What You Use" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -g")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Debug mode")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wall -Wextra -g -O0)

        if(SANITIZER_TYPE STREQUAL "address")
            add_compile_options(-fsanitize=address)
            add_link_options(-fsanitize=address)
        elseif(SANITIZER_TYPE STREQUAL "thread")
            add_compile_options(-fsanitize=thread -fPIE -pie)
            add_link_options(-fsanitize=thread)
        endif()
    endif()
endif()

if(ENABLE_IWYU)
    find_program(IWYU_PATH NAMES include-what-you-use iwyu)
    if(IWYU_PATH)
        set(IWYU_MAPPING_FILE "${CMAKE_SOURCE_DIR}/iwyu.imp")
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--mapping_file=${IWYU_MAPPING_FILE}")
    else()
        message(FATAL_ERROR "include-what-you-use not found")
    endif()
endif()

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_PATH NAMES clang-tidy)
    if(CLANG_TIDY_PATH)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_PATH})
    else()
        message(FATAL_ERROR "clang-tidy not found")
    endif()
endif()

# RD_PARTY_LINK_DIR is precompiled third party libraries
set(THIRD_PARTY_INSTALL_DIR
    "$ENV{THIRD_PARTY_INATALL_DIR}"
    CACHE PATH "Third party libraries install directory")
if(THIRD_PARTY_INSTALL_DIR)
    link_directories(${THIRD_PARTY_INSTALL_DIR}/lib)
    link_directories(${THIRD_PARTY_INSTALL_DIR}/lib64)
    include_directories(${THIRD_PARTY_INSTALL_DIR}/include)
    # hints for cmake to find openssl
    set(OpenSSL_DIR ${THIRD_PARTY_INSTALL_DIR}/lib64/cmake/OpenSSL)
    set(OPENSSL_INCLUDE_DIR ${THIRD_PARTY_INSTALL_DIR}/include)
endif()

find_package(OpenSSL REQUIRED CONFIG)
if (ENABLE_GPU)
    find_package(CUDAToolkit REQUIRED)
endif()
find_package(RapidJSON REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(HWLOC REQUIRED hwloc)

if(trace_agent_ABSL_PROVIDER STREQUAL "module")
    set(ABSL_PROPAGATE_CXX_STD ON)
    set(ABSL_ENABLE_INSTALL ON)
    add_subdirectory(third_party/abseil-cpp)
    include_directories(third_party/abseil-cpp)
else()
    find_package(absl REQUIRED CONFIG)
endif()

if(trace_agent_GRPC_PROVIDER STREQUAL "module")
    set(gRPC_BUILD_TESTS OFF)
    set(gRPC_USE_SYSTEMD
        "OFF"
        CACHE STRING "Build with libsystemd support if available. Can be ON, OFF or AUTO")
    set(gRPC_USE_SYSTEMD OFF)
    set(gRPC_INSTALL
        ON
        CACHE BOOL "Enable grpc install")
    add_subdirectory("third_party/grpc")
    include_directories("third_party/grpc/include")
else()
    find_package(gRPC CONFIG REQUIRED)
endif()

if(trace_agent_PROTOBUF_PROVIDER STREQUAL "module")
    if(TARGET protobuf::libprotobuf)
        include_directories(${protobuf_SOURCE_DIR}/src)
    else()
        set(protobuf_BUILD_TESTS OFF)
        add_subdirectory(third_party/protobuf)
    endif()
    set(_trace_agent_PROTOBUF_PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc>)
else()
    find_package(Protobuf CONFIG REQUIRED)
    include_directories(${Protobuf_INCLUDE_DIRS})
    find_program(
        _trace_agent_PROTOBUF_PROTOC_EXECUTABLE protoc
        HINTS ${_trace_agent_PROTOBUF_PROTOC_EXECUTABLE_PATH}
        PATH_SUFFIXES bin
        DOC "Path to the protoc executable")
    message(STATUS "protoc:${_trace_agent_PROTOBUF_PROTOC_EXECUTABLE}")
endif()

if(trace_agent_CPU_FEATURES_PROVIDER STREQUAL "module")
    add_subdirectory(third_party/cpu_features)
    include_directories("third_party/cpu_features/include")
else()
    find_package(CpuFeatures REQUIRED)
endif()

enable_testing()
find_package(GTest CONFIG REQUIRED)

include(cmake/opentelemetry-proto.cmake)
include(cmake/skywalking-data-collect-protocol.cmake)
include(cmake/deep-agent-payload-protocol.cmake)
include(cmake/cri-api.cmake)

if(trace_agent_OPENTELEMETRY_CPP_PROVIDER STREQUAL "module")
    set(WITH_HTTP_CLIENT_CURL OFF)
    set(BUILD_TESTING OFF)
    set(WITH_OTLP_GRPC OFF)
    set(WITH_ABSEIL ON)
    add_subdirectory(third_party/opentelemetry-cpp)
else()
    find_package(opentelemetry-cpp CONFIG REQUIRED)
endif()

if(trace_agent_STDUUID_PROVIDER STREQUAL "module")
    add_subdirectory(third_party/stduuid)
else()
    find_package(stduuid CONFIG REQUIRED)
endif()

if(trace_agent_LIBCORE_PROVIDER STREQUAL "module")
    set(libcore_FMT_PROVIDER "module")
    set(libcore_LIBEVENT_PROVIDER "module")
endif()
add_compile_definitions(SPDLOG_FMT_EXTERNAL)
add_subdirectory(third_party/libcore)

file(GLOB_RECURSE APP_SRCS "app/*.cc")
include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/app/include")

include_directories("${PROJECT_SOURCE_DIR}/third_party/libcore/core/include")

find_package(spdlog CONFIG REQUIRED)
add_subdirectory(proto)
add_subdirectory(app)
add_subdirectory(tests)

message(STATUS "APP_SRCS:${APP_SRCS}")
add_executable(${APP_NAME} main.cc ${APP_SRCS}
        app/include/source/http/health_check.h
        app/include/source/http/source.h)

target_link_libraries(
    ${APP_NAME}
    PRIVATE trace_agent_opentelemetry_proto
            skywalking_data_collect_protocol_proto
            node_data_collect_protocol_proto
            trace_agent_config_proto
            cri_api_proto
            core
            uuid
            stduuid
            absl::strings
            absl::base
            absl::algorithm
            absl::hash
            absl::flags
            absl::flags_parse
            CpuFeatures::cpu_features)

if(trace_agent_OPENTELEMETRY_CPP_PROVIDER STREQUAL "module")
    target_link_libraries(${APP_NAME} PRIVATE opentelemetry_ext opentelemetry_metrics opentelemetry_trace
                                              opentelemetry_resources)
else()
    message("${OPENTELEMETRY_CPP_LIBRARIES}")
    target_link_libraries(
        ${APP_NAME}
        PRIVATE opentelemetry-cpp::common
                opentelemetry-cpp::trace
                opentelemetry-cpp::resources
                opentelemetry-cpp::sdk
                opentelemetry-cpp::otlp_grpc_exporter
                opentelemetry-cpp::otlp_grpc_metrics_exporter
                opentelemetry-cpp::in_memory_span_exporter
                opentelemetry-cpp::metrics)
endif()

if(trace_agent_GRPC_PROVIDER STREQUAL "module")
    target_link_libraries(${APP_NAME} PRIVATE grpc++ grpc grpc++_reflection grpc++_unsecure)
else()
    target_link_libraries(${APP_NAME} PRIVATE gRPC::grpc++ gRPC::grpc gRPC::grpc++_reflection gRPC::grpc++_unsecure)
endif()

if(trace_agent_ABSL_PROVIDER STREQUAL "module")
    target_link_libraries(${APP_NAME} PRIVATE strings base algorithm hash)
else()
    target_link_libraries(${APP_NAME} PRIVATE absl::strings absl::base absl::algorithm absl::hash)
endif()

if (ENABLE_GPU)
target_link_libraries(${APP_NAME} PRIVATE CUDA::nvml)
endif()
find_package(Libevent CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE libevent::core libevent::extra libevent::openssl libevent::pthreads)

target_include_directories(${APP_NAME} PRIVATE ${HWLOC_INCLUDE_DIRS})
target_compile_options(${APP_NAME} PRIVATE ${HWLOC_CFLAGS})
target_link_libraries(${APP_NAME} PRIVATE ${HWLOC_LINK_LIBRARIES})
target_link_options(${APP_NAME} PRIVATE ${HWLOC_LDFLAGS})

find_package(LibXml2 CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE LibXml2::LibXml2)

find_package(prometheus-cpp CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE prometheus-cpp::core prometheus-cpp::pull)
