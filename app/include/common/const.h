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

typedef enum {
    UNDEFINED,
    MESH,
    GENERAL,
    OS_LINUX,
    K8S,
    FAAS,
    MESH_CP,
    MESH_DP,
    DATABASE,
    CACHE,
    BROWSER,
    SO11Y_OAP,
    SO11Y_SATELLITE,
    MQ,
    VIRTUAL_DATABASE,
    VIRTUAL_MQ,
    VIRTUAL_GATEWAY,
    K8S_SERVICE,
    MYSQL,
    VIRTUAL_CACHE,
    POSTGRESQL,
    APISIX,
    AWS_EKS,
    OS_WINDOWS,
    AWS_S3,
    AWS_DYNAMODB,
    AWS_GATEWAY,
    REDIS,
    ELASTICSEARCH,
    RABBITMQ,
    MONGODB,
    KAFKA,
    PULSAR,
    BOOKKEEPER,
    NGINX,
    ROCKETMQ,
    CLICKHOUSE,
    ACTIVEMQ,
    CILIUM_SERVICE,
    SO11Y_JAVA_AGENT,
    KONG
} LAYER;

/**
 * RPC request type.
 */
typedef enum {
    DATABASE_REQ,
    HTTP,
    RPC,
    gRPC,
    /**
     * Logic request only.
     */
    LOGIC,
    TCP,
    MQ_REQ
} RequestType;

namespace Skywalking {
const static std::string skywalking_grpc = "skywalking-grpc";
static std::string AttributeSkywalkingSegmentID = "sw8.segment_id";
static std::string AttributeRefType = "refType";
static std::string  AttributeParentService             = "parent.service";
static std::string  AttributeParentInstance            = "parent.service.instance";
static std::string  AttributeParentEndpoint            = "parent.endpoint";
static std::string  AttributeSkywalkingSpanID          = "sw8.span_id";
static std::string AttributeSkywalkingTraceID         = "sw8.trace_id";
static std::string AttributeSkywalkingParentSpanID    = "sw8.parent_span_id";
static std::string AttributeSkywalkingParentSegmentID = "sw8.parent_segment_id";
static std::string AttributeNetworkAddressUsedAtPeer  = "network.AddressUsedAtPeer";
static std::string AttributePeer = "sw8.peer";
static std::string AttributeComponentId = "sw8.componentId";
static std::string AttributeLayer = "sw8.layer";
}

static std::string  LogicEndpoint = "x-le";

namespace Topology {
static std::string UserEndpointName = "User";
static std::string UserInstanceName = "User";
static std::string UserPointName = "User";
static std::string UserServiceName = "User";

enum DetectPoint {
    SERVER = 0,
    CLIENT = 1,
    PROXY = 2,
    UNRECOGNIZED = 3
};
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
