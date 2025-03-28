//
// Created by zhanglei on 25-2-3.
//

#pragma once

namespace App::Source::Host::Collector::Network {
#define IPX_NODE_LEN	6
#define IPX_MTU		576

struct sockaddr_ipx {
#if LINUX_VERSION_CODE > 131328	/* 2.1.0 or later */
    sa_family_t sipx_family;
#else
    short sipx_family;
#endif
    unsigned short sipx_port;
    unsigned int sipx_network;
    unsigned char sipx_node[IPX_NODE_LEN];
    unsigned char sipx_type;
    unsigned char sipx_zero;	/* 16 byte fill */
};

#define IPX_FRAME_NONE		0
#define IPX_FRAME_SNAP		1
#define IPX_FRAME_8022		2
#define IPX_FRAME_ETHERII	3
#define IPX_FRAME_8023		4
#define IPX_FRAME_TR_8022	5

#define IPV6_ADDR_ANY		0x0000U

#define IPV6_ADDR_UNICAST      	0x0001U
#define IPV6_ADDR_MULTICAST    	0x0002U
#define IPV6_ADDR_ANYCAST	0x0004U

#define IPV6_ADDR_LOOPBACK	0x0010U
#define IPV6_ADDR_LINKLOCAL	0x0020U
#define IPV6_ADDR_SITELOCAL	0x0040U

#define IPV6_ADDR_COMPATv4	0x0080U

#define IPV6_ADDR_SCOPE_MASK	0x00f0U

#define IPV6_ADDR_MAPPED	0x1000U
#define IPV6_ADDR_RESERVED	0x2000U
}