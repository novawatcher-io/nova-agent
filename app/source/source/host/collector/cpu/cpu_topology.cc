#include "app/include/source/host/collector/cpu/cpu_topology.h"
#include <cstring>
#include <hwloc.h>
#include <spdlog/spdlog.h>
#include <string>
#include <zlib.h>
namespace App::Source::Host::Collector::Cpu {
std::string gzipCompress(const std::string& data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    // Initialize the zlib stream for compression
    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        SPDLOG_ERROR("Failed to initialize zlib compression.");
        return "";
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
    zs.avail_in = data.size();

    int ret;
    char outbuffer[32768]; // Output buffer
    std::string outstring;

    // Compress the data
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        SPDLOG_ERROR("Zlib compression failed.");
        return "";
    }

    return outstring;
}

std::string CPUTopology::GetTopologyXML() {
    // Initialize the topology object
    hwloc_topology_t topology = nullptr;
    hwloc_topology_init(&topology);

    // Load the current system's topology
    hwloc_topology_load(topology);

    char* buffer = nullptr;
    int len = 0;
    unsigned long const flags = 0;
    int const ret = hwloc_topology_export_xmlbuffer(topology, &buffer, &len, flags);
    if (ret != 0) {
        SPDLOG_ERROR("Failed to export topology to XML buffer");
        return "";
    }
    std::string result(buffer, len);
    hwloc_free_xmlbuffer(topology, buffer);
    return result;
}

void CPUTopology::GetCPUTopology(std::string& topology) {
    auto xml = GetTopologyXML();
    topology = gzipCompress(xml);
}
} // namespace App::Source::Host::Collector::Cpu
