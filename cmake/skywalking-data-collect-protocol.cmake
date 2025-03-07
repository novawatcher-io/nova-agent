set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/third_party/skywalking-data-collect-protocol")
set(PROTO_PATH "${INSTALL_DIR}")
set(PROTOBUF_RUN_PROTOC_COMMAND "\"${PROTOBUF_PROTOC_EXECUTABLE}\"")

set(BROWSERPERF_PROTO
        "${PROTO_PATH}/browser/BrowserPerf.proto")
set(BROWSERPERFCOMPAT_PROTO
        "${PROTO_PATH}/browser/BrowserPerfCompat.proto")
set(COMMON_PROTO "${PROTO_PATH}/common/Common.proto")
set(COMMAND_PROTO "${PROTO_PATH}/common/Command.proto")
set(EBPF_PROFILING_CONTINUOUS_PROTO "${PROTO_PATH}/ebpf/profiling/Continuous.proto")
set(EBPF_PROFILING_PROCESS_PROTO "${PROTO_PATH}/ebpf/profiling/Process.proto")
set(EBPF_PROFILING_PROFILE_PROTO "${PROTO_PATH}/ebpf/profiling/Profile.proto")
set(EBPF_ACCESSLOG_PROTO "${PROTO_PATH}/ebpf/accesslog.proto")
set(EBPF_EVENT_EVENT_PROTO "${PROTO_PATH}/event/Event.proto")
set(LANGUAGE_AGENT_CLRMETRIC_PROTO "${PROTO_PATH}/language-agent/CLRMetric.proto")
set(LANGUAGE_AGENT_CLRMETRICCOMPAT_PROTO "${PROTO_PATH}/language-agent/CLRMetricCompat.proto")
set(LANGUAGE_AGENT_CONFIGURATIONDISCOVERYSERVICE_PROTO "${PROTO_PATH}/language-agent/ConfigurationDiscoveryService.proto")
set(LANGUAGE_AGENT_JVMMETRIC_PROTO "${PROTO_PATH}/language-agent/JVMMetric.proto")
set(LANGUAGE_AGENT_JVMMETRICCOMPAT_PROTO "${PROTO_PATH}/language-agent/JVMMetricCompat.proto")
set(LANGUAGE_AGENT_METER_PROTO "${PROTO_PATH}/language-agent/Meter.proto")
set(LANGUAGE_AGENT_METERCOMPAT_PROTO "${PROTO_PATH}/language-agent/MeterCompat.proto")
set(LANGUAGE_AGENT_TRACING_PROTO "${PROTO_PATH}/language-agent/Tracing.proto")
set(LANGUAGE_AGENT_TRACINGCOMPAT_PROTO "${PROTO_PATH}/language-agent/TracingCompat.proto")
set(LOGGING_LOGGING_PROTO "${PROTO_PATH}/logging/Logging.proto")
set(MANAGEMENT_MANAGEMENT_PROTO "${PROTO_PATH}/management/Management.proto")
set(MANAGEMENT_MANAGEMENTCOMPAT_PROTO "${PROTO_PATH}/management/ManagementCompat.proto")
set(PROFILE_PROFILE_PROTO "${PROTO_PATH}/profile/Profile.proto")
set(PROFILE_PROFILECOMPAT_PROTO "${PROTO_PATH}/profile/ProfileCompat.proto")
set(SERVICE_MESH_PROBE_SERVICE_MESH_PROTO "${PROTO_PATH}/service-mesh-probe/service-mesh.proto")

set(GENERATED_PROTOBUF_PATH
        "${CMAKE_BINARY_DIR}/generated/third_party/skywalking-data-collect-protocol")

message(STATUS "${GENERATED_PROTOBUF_PATH}")

file(MAKE_DIRECTORY "${GENERATED_PROTOBUF_PATH}")

set(BROWSER_BROWSERPERF_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/browser/BrowserPerf.pb.cc")
set(BROWSER_BROWSERPERF_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/browser/BrowserPerf.pb.h")
set(BROWSER_BROWSERPERFCOMPAT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/browser/BrowserPerfCompat.pb.cc")
set(BROWSER_BROWSERPERFCOMPAT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/browser/BrowserPerfCompat.pb.h")
set(COMMON_COMMON_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/common/Common.pb.cc")
set(COMMON_COMMON_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/common/Common.pb.h")
set(COMMON_COMMAND_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/common/Command.pb.cc")
set(COMMON_COMMAND_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/common/Command.pb.h")
set(EBPF_PROFILING_CONTINUOUS_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/ebpf/profiling/Continuous.pb.cc")
set(EBPF_PROFILING_CONTINUOUS_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/ebpf/profiling/Continuous.pb.h")
set(EBPF_PROFILING_PROCESS_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/ebpf/profiling/Process.pb.cc")
set(EBPF_PROFILING_PROCESS_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/ebpf/profiling/Process.pb.h")
set(EBPF_PROFILING_PROFILE_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/ebpf/profiling/Profile.pb.cc")
set(EBPF_PROFILING_PROFILE_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/ebpf/profiling/Profile.pb.h")
set(EBPF_ACCESSLOG_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/ebpf/accesslog.pb.cc")
set(EBPF_ACCESSLOG_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/ebpf/accesslog.pb.h")
set(EVENT_EVENT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/event/Event.pb.cc")
set(EVENT_EVENT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/event/Event.pb.h")
set(LANGUAGE_AGENT_CLRMETRIC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/CLRMetric.pb.cc"
        )
set(LANGUAGE_AGENT_CLRMETRIC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/CLRMetric.pb.h"
        )
set(LANGUAGE_AGENT_CLRMETRICCOMPAT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/CLRMetricCompat.pb.cc"
        )
set(LANGUAGE_AGENT_CLRMETRICCOMPAT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/CLRMetricCompat.pb.h"
        )
set(LANGUAGE_AGENT_CONFIGURATIONDISCOVERYSERVICE_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/ConfigurationDiscoveryService.pb.cc"
        )
set(LANGUAGE_AGENT_CONFIGURATIONDISCOVERYSERVICE_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/ConfigurationDiscoveryService.pb.h"
        )
set(LANGUAGE_AGENT_JVMMETRIC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/JVMMetric.pb.cc"
        )
set(LANGUAGE_AGENT_JVMMETRIC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/JVMMetric.pb.h"
        )
set(LANGUAGE_AGENT_JVMMETRICCOMPAT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/JVMMetricCompat.pb.cc"
        )
set(LANGUAGE_AGENT_JVMMETRICCOMPAT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/JVMMetricCompat.pb.h"
        )
set(LANGUAGE_AGENT_METER_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/Meter.pb.cc"
        )
set(LANGUAGE_AGENT_METER_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/Meter.pb.h"
        )
set(LANGUAGE_AGENT_METERCOMPAT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/MeterCompat.pb.h"
        )
set(LANGUAGE_AGENT_METERCOMPAT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/MeterCompat.pb.cc"
        )
set(LANGUAGE_AGENT_TRACING_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/Tracing.pb.cc"
        )
set(LANGUAGE_AGENT_TRACING_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/Tracing.pb.h"
        )
set(LANGUAGE_AGENT_TRACINGCOMPAT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/TracingCompat.pb.cc"
        )
set(LANGUAGE_AGENT_TRACINGCOMPAT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/TracingCompat.pb.h"
        )
set(LOGGING_LOGGING_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/logging/Logging.pb.cc"
        )
set(LOGGING_LOGGING_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/logging/Logging.pb.h"
        )
set(MANAGEMENT_MANAGEMENT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/management/Management.pb.cc"
        )
set(MANAGEMENT_MANAGEMENT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/management/Management.pb.h"
        )
set(MANAGEMENT_MANAGEMENTCOMPAT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/management/ManagementCompat.pb.cc"
        )
set(MANAGEMENT_MANAGEMENTCOMPAT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/management/ManagementCompat.pb.h"
        )
set(PROFILE_PROFILE_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/profile/Profile.pb.cc"
        )
set(PROFILE_PROFILE_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/profile/Profile.pb.h"
        )
set(PROFILE_PROFILECOMPAT_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/profile/ProfileCompat.pb.cc"
        )
set(PROFILE_PROFILECOMPAT_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/profile/ProfileCompat.pb.h"
        )
set(SERVICE_MESH_PROBE_SERVICE_MESH_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/service-mesh-probe/service-mesh.pb.cc"
        )
set(SERVICE_MESH_PROBE_SERVICE_MESH_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/service-mesh-probe/service-mesh.pb.h"
        )
set(TRACING_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/Tracing.grpc.pb.cc"
        )
set(CLRMetric_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/CLRMetric.grpc.pb.cc"
)
set(JVMMETRIC_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/JVMMetric.grpc.pb.h"
        )
set(METER_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/Meter.grpc.pb.cc"
        )
set(METER_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/Meter.grpc.pb.h"
        )
set(JVMMETRIC_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/JVMMetric.grpc.pb.cc"
        )
set(TRACING_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/Tracing.grpc.pb.h"
        )
set(CONFIGURATION_DISCOVERY_SERVICE_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/ConfigurationDiscoveryService.grpc.pb.cc"
        )
set(CONFIGURATION_DISCOVERY_SERVICE_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/language-agent/ConfigurationDiscoveryService.grpc.pb.h"
        )
set(MANAGEMENT_MANAGEMENT_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/management/Management.grpc.pb.cc"
        )
set(MANAGEMENT_MANAGEMENT_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/management/Management.grpc.pb.h"
        )
set(LOGGING_LOGGING_GRPC_PB_CPP_FILE
        "${GENERATED_PROTOBUF_PATH}/logging/Logging.grpc.pb.cc"
        )
set(LOGGING_LOGGING_GRPC_PB_H_FILE
        "${GENERATED_PROTOBUF_PATH}/logging/Logging.grpc.pb.h"
        )

set(SKYWALKING_PROTOBUF_GENERATED_FILES
        ${BROWSER_BROWSERPERF_PB_CPP_FILE}
        ${BROWSER_BROWSERPERF_PB_H_FILE}
        ${BROWSER_BROWSERPERFCOMPAT_PB_CPP_FILE}
        ${BROWSER_BROWSERPERFCOMPAT_PB_H_FILE}
        ${COMMON_COMMON_PB_CPP_FILE}
        ${COMMON_COMMON_PB_H_FILE}
        ${COMMON_COMMAND_PB_CPP_FILE}
        ${COMMON_COMMAND_PB_H_FILE}
        ${EBPF_PROFILING_CONTINUOUS_PB_CPP_FILE}
        ${EBPF_PROFILING_CONTINUOUS_PB_H_FILE}
        ${EBPF_PROFILING_PROCESS_PB_CPP_FILE}
        ${EBPF_PROFILING_PROCESS_PB_H_FILE}
        ${EBPF_PROFILING_PROFILE_PB_CPP_FILE}
        ${EBPF_PROFILING_PROFILE_PB_H_FILE}
        ${EBPF_ACCESSLOG_PB_CPP_FILE}
        ${EBPF_ACCESSLOG_PB_H_FILE}
        ${EVENT_EVENT_PB_CPP_FILE}
        ${EVENT_EVENT_PB_H_FILE}
        ${LANGUAGE_AGENT_CLRMETRIC_PB_CPP_FILE}
        ${LANGUAGE_AGENT_CLRMETRIC_PB_H_FILE}
        ${LANGUAGE_AGENT_CLRMETRICCOMPAT_PB_CPP_FILE}
        ${LANGUAGE_AGENT_CLRMETRICCOMPAT_PB_H_FILE}
        ${LANGUAGE_AGENT_CONFIGURATIONDISCOVERYSERVICE_PB_CPP_FILE}
        ${LANGUAGE_AGENT_CONFIGURATIONDISCOVERYSERVICE_PB_H_FILE}
        ${LANGUAGE_AGENT_JVMMETRIC_PB_CPP_FILE}
        ${LANGUAGE_AGENT_JVMMETRIC_PB_H_FILE}
        ${LANGUAGE_AGENT_JVMMETRICCOMPAT_PB_CPP_FILE}
        ${LANGUAGE_AGENT_JVMMETRICCOMPAT_PB_H_FILE}
        ${LANGUAGE_AGENT_METER_PB_CPP_FILE}
        ${LANGUAGE_AGENT_METER_PB_H_FILE}
        ${LANGUAGE_AGENT_METERCOMPAT_PB_H_FILE}
        ${LANGUAGE_AGENT_METERCOMPAT_PB_CPP_FILE}
        ${LANGUAGE_AGENT_TRACING_PB_CPP_FILE}
        ${LANGUAGE_AGENT_TRACING_PB_H_FILE}
        ${LANGUAGE_AGENT_TRACINGCOMPAT_PB_CPP_FILE}
        ${LANGUAGE_AGENT_TRACINGCOMPAT_PB_H_FILE}
        ${LOGGING_LOGGING_PB_CPP_FILE}
        ${LOGGING_LOGGING_PB_H_FILE}
        ${MANAGEMENT_MANAGEMENT_PB_CPP_FILE}
        ${MANAGEMENT_MANAGEMENT_PB_H_FILE}
        ${MANAGEMENT_MANAGEMENTCOMPAT_PB_CPP_FILE}
        ${MANAGEMENT_MANAGEMENTCOMPAT_PB_H_FILE}
        ${PROFILE_PROFILE_PB_CPP_FILE}
        ${PROFILE_PROFILE_PB_H_FILE}
        ${PROFILE_PROFILECOMPAT_PB_CPP_FILE}
        ${PROFILE_PROFILECOMPAT_PB_H_FILE}
        ${SERVICE_MESH_PROBE_SERVICE_MESH_PB_CPP_FILE}
        ${SERVICE_MESH_PROBE_SERVICE_MESH_PB_H_FILE})

foreach(IMPORT_DIR ${PROTOBUF_IMPORT_DIRS})
    list(APPEND PROTOBUF_INCLUDE_FLAGS "-I${IMPORT_DIR}")
endforeach()

include_directories("${CMAKE_BINARY_DIR}/generated/third_party")

message(STATUS "${COMMON_PROTO} ${RESOURCE_PROTO} ${TRACE_PROTO}
        ${LOGS_PROTO} ${METRICS_PROTO} ${TRACE_SERVICE_PROTO} ${LOGS_SERVICE_PROTO}
        ${METRICS_SERVICE_PROTO}")

if(nova_agent_GRPC_PROVIDER STREQUAL "package")
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
        ${CLRMetric_GRPC_PB_CPP_FILE}
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
        ${_nova_agent_PROTOBUF_PROTOC_EXECUTABLE} ${PROTOBUF_COMMON_FLAGS}
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

add_library(
        skywalking_data_collect_protocol_proto
        ${SKYWALKING_PROTOBUF_GENERATED_FILES}
)

foreach(file ${SKYWALKING_PROTOBUF_GENERATED_FILES})
        set_source_files_properties(${file} PROPERTIES SKIP_LINTING ON)
endforeach()
