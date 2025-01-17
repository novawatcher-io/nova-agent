#pragma once

#include <string>
namespace App::Source::Host::Collector::Container {
struct ContainerInfo {
    std::string name;
    std::string id;
    std::string state;
    std::string command;
    int64_t created = 0;
    int pid = 0;
};
} // namespace App::Source::Host::Collector::Container