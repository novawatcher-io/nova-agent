//
// Created by zhanglei on 25-2-1.
//

#pragma once

namespace App::Source::Host::Collector::Network {
#define E_NOTFOUND	8
#define E_SOCK		7
#define E_LOOKUP	6
#define E_VERSION	5
#define E_USAGE		4
#define E_OPTERR	3
#define E_INTERN	2
#define E_NOSUPP	1

#define RTACTION_ADD   1
#define RTACTION_DEL   2
#define RTACTION_HELP  3
#define RTACTION_FLUSH 4
#define RTACTION_SHOW  5

#define FLAG_EXT       3		/* AND-Mask */
#define FLAG_NUM_HOST  4
#define FLAG_NUM_PORT  8
#define FLAG_NUM_USER 16
#define FLAG_NUM     (FLAG_NUM_HOST|FLAG_NUM_PORT|FLAG_NUM_USER)
#define FLAG_SYM      32
#define FLAG_CACHE    64
#define FLAG_FIB     128
#define FLAG_VERBOSE 256

#define RTF_EXPIRES     0x00400000
}