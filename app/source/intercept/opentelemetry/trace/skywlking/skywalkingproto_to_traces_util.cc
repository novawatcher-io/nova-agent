#include "app/include/intercept/opentelemetry/trace/skywalking/skywalkingproto_to_traces_util.h"
#include <absl/numeric/int128.h>         // for uint128, int128, Uint128High64
#include <string.h>                      // for memcpy
#include <uuid/uuid.h>                   // for uuid_parse, uuid_t
#include <array>                         // for array
#include <utility>                       // for pair
#include "common/hex.h"                  // for isUint128, HexDecode
#include "common/opentelemetry/const.h"  // for SpanID, TraceID

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Trace {
namespace Skywalking {
Common::Opentelemetry::TraceID SkywalkingProtoToTracesUtil::swTraceIdToTraceId(const std::string &trace_id, opentelemetry::nostd::span<uint8_t, 16>& ret) {
    auto uint128Ret = Common::isUint128(trace_id);
    if (uint128Ret.second) {
        // php-agent
        absl::uint128 value = static_cast<absl::int128>(uint128Ret.first);
        uint64_t low = absl::Uint128Low64(value);
        uint64_t high = absl::Uint128High64(value);
        std::array<uint8_t, 16> bytes;
        memcpy(bytes.data(), &low, sizeof(low));
        memcpy(bytes.data() + sizeof(low), &high, sizeof(high));
        return {bytes};
    }
    if (trace_id.length() <= 36) {
        uuid_t u;
        // de5980b8-fce3-4a37-aab9-b4ac3af7eedd: from browser/js-sdk/envoy/nginx-lua sdk/py-agent
        int result = uuid_parse(trace_id.c_str(), u);
        if (result) {
            // 56a5e1c519ae4c76a2b8b11d92cead7f
            if (!swStringToUUID(trace_id, 0, ret)) {
                return {nullptr, 16};
            }
            return ret;
        }
        return {u};
    }
    // 56a5e1c519ae4c76a2b8b11d92cead7f.12.16563474296430001: from java-agent
    if (!swStringToUUID(trace_id, 0, ret)) {
        return {nullptr, 16};
    }
    return ret;
}

Common::Opentelemetry::SpanID SkywalkingProtoToTracesUtil::segmentIDToSpanID(const std::string &segmentId, uint32_t spanId, Common::Opentelemetry::SpanID& ret) {
    if (segmentId.length() < 32)  {
        return {nullptr, 16};
    }

    auto uuid_ = std::array<uint8_t, 16>();
    nostd::span<uint8_t, 16> uuid(uuid_);

    // 检查是否是uint128
    auto uint128Ret = Common::isUint128(segmentId);
    if (uint128Ret.second) {
        // php-agent
        absl::uint128 value = static_cast<absl::int128>(uint128Ret.first);
        swUint128tToUUID(value, spanId, uuid);
        ret = uuidTo8Bytes(uuid, ret);
        return ret;
    }


    if (! swStringToUUID(segmentId, spanId, uuid)) {
        return ret;
    }
    ret = uuidTo8Bytes(uuid, ret);
    return ret;
}

bool SkywalkingProtoToTracesUtil::swUint128tToUUID(absl::uint128 value, uint32_t extra, opentelemetry::nostd::span<uint8_t, 16>& uid) {
    for (int i = 0; i < 4; i++) {
        uid[i] ^= uint8_t(extra);
        extra >>= 8;
    }

    uint64_t low = absl::Uint128Low64(value);
    uint64_t high = absl::Uint128High64(value);


    for (int i = 4; i < 8; i++) {
        uid[i] ^= uint8_t(low);
        low >>= 8;
    }

    for (int i = 8; i < 16; i++) {
        uid[i] ^= uint8_t(high);
        high >>= 8;
    }
    return true;
}

bool SkywalkingProtoToTracesUtil::swStringToUUID(const std::string &s, uint32_t extra, opentelemetry::nostd::span<uint8_t, 16>& uid) {
    if (s.length() < 32) {
        return false;
    }

    // there are 2 possible formats for 's':
	// s format = 56a5e1c519ae4c76a2b8b11d92cead7f.0000000000.000000000000000000
	//            ^ start(length=32)               ^ mid(u32) ^ last(u64)
	// uid = UUID(start) XOR ([4]byte(extra) . [4]byte(uint32(mid)) . [8]byte(uint64(last)))

	// s format = 56a5e1c519ae4c76a2b8b11d92cead7f
	//            ^ start(length=32)
	// uid = UUID(start) XOR [4]byte(extra)
    auto sub = s.substr(0, 32);
    App::Common::HexDecode(sub, uid);

    for (int i = 0; i < 4; i++) {
        uid[i] ^= uint8_t(extra);
        extra >>= 8;
    }

    if (s.length() == 32) {
        return true;
    }

    size_t index1 = s.find('.');
    size_t index2 = s.rfind('.');
    if (index1 != 32 || index2 == std::string::npos) {
        return false;
    }

    auto size = index2 - index1;
    if (size > 0) {
        size -= 1;
    }

    auto midStr = s.substr(index1+1, size);
    long mid = std::stol(midStr);

    auto lastStr = s.substr(index2+1, s.length());
    long last = std::stol(lastStr);


    for (int i = 4; i < 8; i++) {
        uid[i] ^= uint8_t(mid);
        mid >>= 8;
    }

    for (int i = 8; i < 16; i++) {
        uid[i] ^= uint8_t(last);
        last >>= 8;
    }

    return true;
}
}
}
}
}
}
