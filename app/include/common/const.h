//
// Created by zhanglei on 25-1-12.
//

#pragma once

#include <string>

namespace App::Common {

const static std::string stdout = "stdout";
const static std::string stdout_path = "/tmp/stdout.log";
const static std::string stderr_path = "/tmp/stderr.log";

namespace Log {
const static std::string info = "info";
}


namespace Trace {
namespace Skywalking {
const static std::string skywalking_grpc = "skywalking-grpc";
}
}

namespace Http {
const static std::string http = "http";
const static std::string health_path = "/health";
const static std::string metric_path = "/metrics";
}

namespace Container {
const static std::string docker = "docker";
}

const static std::string uptime_proc_file =  "/proc/uptime";

}
