#ifndef OSPF_CONSTANTS_H
#define OSPF_CONSTANTS_H

#include "ns3/timer.h"
#include "ns3/ipv6-address.h"

namespace ns3 {
namespace ospf {

static const int32_t g_lsRefreshTime = 1800;
static const Time g_minLsInterval = Seconds(5.0);
static const Time g_minLsArrival = Seconds(1.0);
static const uint32_t g_maxAge = 3600; // seconds
static const uint32_t g_checkAge = 300; // seconds
static const int32_t g_maxAgeDiff = 900; // seconds
static const uint32_t g_lsInfinity = 0xffffff;
static const Ipv6Address g_defaultDestination;
static const int32_t g_initialSeqNum = 0x80000001;
static const int32_t g_maxSeqNum = 0x7fffffff;

}
}

#endif
