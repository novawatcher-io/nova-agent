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
    Common::mount_entry* entry = fileSystemList.get();
    while (entry) {
        auto file_system = info->add_file_system_infos();
        file_system->set_available(entry->available);
        file_system->set_size(entry->size);
        file_system->set_used(entry->used);
        file_system->set_name(entry->me_devname);
        file_system->set_type(entry->me_type);
        file_system->set_monit_point(entry->me_mountdir);
        entry = fileSystemList->me_next.get();
    }
    auto file_systems = info->add_file_system_infos();
    return;
}


}