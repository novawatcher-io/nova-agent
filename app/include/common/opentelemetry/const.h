//
// Created by root on 2024/4/1.
//

#pragma once
#include <vector>
#include "opentelemetry/nostd/span.h"
#include "opentelemetry/nostd/unique_ptr.h"
#include "opentelemetry/sdk/instrumentationscope/instrumentation_scope.h"

namespace App {
namespace Common {
namespace Opentelemetry {
using SpanID = opentelemetry::nostd::span<uint8_t, 8>;
using TraceID = opentelemetry::nostd::span<uint8_t, 16>;

const static std::string agent_name_string = "agent_name";
const static std::string agent_version_string = "agent_version";
const static std::string agent_name = "trace-agent";
const static std::string agent_version = "0.1";
const static std::string description = "description";
static std::string AttributeServiceName = "service.name";
static std::string AttributeServiceInstanceID = "service.instance.id";

static opentelemetry::nostd::unique_ptr<opentelemetry::sdk::instrumentationscope::InstrumentationScope> instrumentationScope = opentelemetry::sdk::instrumentationscope::InstrumentationScope::Create(
            agent_name,
            agent_version
        );

static std::unordered_map<std::string, std::string> defaultResource = {
    {agent_name_string, agent_name},
    {agent_version_string, agent_version},
};

typedef enum {
    UNKNOW,
    TRACE,
    METRIC,
    LOG
} PIPLINE_TYPE;

namespace Metric {
namespace JVM {
const static std::string memoryUsed = "jvm.memory.used";
const static std::string memoryUsedDescription = "Measure of memory used.";
const static std::string memoryPoolName = "jvm.memory.pool.name";
const static std::string memoryType = "jvm.memory.type";
const static std::string heap = "heap";
const static std::string non_heap = "non_heap";
const static std::string memoryCommitted = "jvm.memory.committed";
const static std::string memoryCommittedDescription = "Measure of memory committed.";
const static std::string memoryUsedAfterLastGc = "jvm.memory.used_after_last_gc";
const static std::string memoryInit = "jvm.memory.init";
const static std::string memoryInitDescription = "Measure of initial memory requested.\t";
const static std::string memoryBufferLimit = "jvm.buffer.memory.limit";
const static std::string memoryBufferCount = "jvm.buffer.count";
const static std::string memoryLimit = "jvm.memory.limit";
const static std::string memoryLimitDescription = "Measure of max obtainable memory.";

const static std::string memoryPoolUse = "jvm.memory.pool.use";
const static std::string memoryPoolInit = "jvm.memory.pool.init";
const static std::string memoryPoolLimit = "jvm.memory.pool.init";
const static std::string memoryPoolCommitted = "jvm.memory.pool.init";
}

namespace GC {
const static std::string gcDuration = "jvm.gc.duration";
const static std::string gcDurationDescription = "Duration of JVM garbage collection actions.";
const static std::string gcAction = "jvm.gc.action";
const static std::string gcName = "jvm.gc.name";
}

namespace JVMThread {
const std::string count = "jvm.thread.count";
const std::string countDescription = "Number of executing platform threads.";
const std::string daemon = "jvm.thread.daemon";
const std::string state = "jvm.thread.state";
}

namespace JVMClasses {
const std::string loaded = "jvm.class.loaded";
const std::string loadedDescription = "Number of classes loaded since JVM start";
const std::string unloaded = "jvm.class.unloaded";
const std::string unloadeddDescription = "Number of classes unloaded since JVM start.";
const std::string count = "jvm.class.count";
const std::string countDescription = "Number of classes currently loaded.";
}

namespace CPU {
const static std::string time = "jvm.cpu.time";
const static std::string count = "jvm.cpu.count";
const static std::string recent_utilization = "jvm.cpu.recent_utilization";
const static std::string recentUtilizationDescription = "Recent CPU utilization for the process as reported by the JVM. [1]";
}


}

}
}
}
