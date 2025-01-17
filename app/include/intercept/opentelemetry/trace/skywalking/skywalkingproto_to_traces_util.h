#pragma once
#include <absl/numeric/int128.h>                     // for uint128
#include <opentelemetry/nostd/span.h>                // for span
#include <stdint.h>                                  // for uint8_t, uint32_t
#include <string>                                    // for string
#include "app/include/common/opentelemetry/const.h"  // for SpanID, TraceID

namespace App {
namespace Intercept {
namespace Opentelemetry {
namespace Trace {
namespace Skywalking {
using namespace opentelemetry;
class SkywalkingProtoToTracesUtil {
public:
    static Common::Opentelemetry::TraceID swTraceIdToTraceId(const std::string& trace_id,
                                                             opentelemetry::nostd::span<uint8_t, 16>& ret);

    static Common::Opentelemetry::SpanID segmentIDToSpanID(const std::string& segmentId, uint32_t spanId,
                                                           Common::Opentelemetry::SpanID& ret);

    static nostd::span<uint8_t, 8> uuidTo8Bytes(nostd::span<uint8_t, 16> uuid, nostd::span<uint8_t, 8>& dst) {
        for (int i = 0; i < 8; i++) {
            dst[i] = uuid[i] ^ uuid[i + 8];
        }
        return dst;
    }

    static bool swUint128tToUUID(absl::uint128 value, uint32_t extra, opentelemetry::nostd::span<uint8_t, 16>& uid);

    static bool swStringToUUID(const std::string& s, uint32_t extra,
                                                   opentelemetry::nostd::span<uint8_t, 16>& uid);
};
}
}
}
}
}
