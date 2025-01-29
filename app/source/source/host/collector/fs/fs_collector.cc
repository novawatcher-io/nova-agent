//
// Created by zhanglei on 25-1-25.
//
#include "app/include/source/host/collector/fs/fs_collector.h"

#include <fstream>
#include <spdlog/spdlog.h>

#include "app/include/common/file.h"

namespace App::Source::Host::Collector::Fs {



void FsCollector::run(novaagent::node::v1::NodeInfo* info) {
    Common::BasicFileReader reader;
    std::unique_ptr<Common::mount_entry> fileSystemList = reader.ReadFileSystemList(false);
    return;
}


}