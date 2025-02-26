//
// Created by zhanglei on 2025/2/25.
//

#pragma once

namespace App::Source::Kubernetes {
const std::string type = "type";


const std::string added = "ADDED";

const std::string modified = "MODIFIED";

const std::string deleted = "DELETED";

const std::string bookmark = "BOOKMARK";

const std::string error = "ERROR";

namespace Kind {
static std::string service = "Service";
static std::string pod = "Pod";
}
}
