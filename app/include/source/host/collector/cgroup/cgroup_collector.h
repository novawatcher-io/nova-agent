#pragma once
#include "common/file.h"
#include "node/v1/info.pb.h"

namespace App::Source::Host::Collector::CGroup {
class CGroupCollector {
public:
    void GetCGroupInfo(::deepagent::node::v1::CGroupInfo& cgroup_info);
    void handleFile(const std::filesystem::directory_entry& entry, deepagent::node::v1::CGroupMount* mount);

private:
    App::Common::BasicFileReader default_reader_;
    void handleHugetlb(const std::string& filename, const std::string& content,
                       deepagent::node::v1::CGroupHugeTLBController* hugetlb);
};
} // namespace App::Source::Host::Collector::CGroup