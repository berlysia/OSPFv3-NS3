#ifndef OSPF_STRUCT_INTERFACE_H
#define OSPF_STRUCT_INTERFACE_H

#include "ns3/timer.h"
#include "ns3/callback.h"
#include "ospf-struct-neighbor.h"
#include "ospf-lsa-header.h"
#include <vector>
#include <map>

namespace ns3 {
namespace ospf {

namespace InterfaceTypeNS {
enum Type {
    P2P = 1,
    BROADCAST = 2,
    NBMA = 3,
    P2M = 4,
    VIRTUAL = 5,
};
}

namespace InterfaceStateNS {
enum Type {
    DOWN = 1,
    P2P = 2,
    LOOPBACK = 3,
    WAITING = 4,
    DROTHER = 5,
    BACKUP = 6,
    DR = 7,
};
}

typedef InterfaceTypeNS::Type InterfaceType;
typedef InterfaceStateNS::Type InterfaceState;
    
struct InterfaceData {
    typedef uint32_t RouterId;
    InterfaceType m_type;
    InterfaceState m_state;
    uint32_t m_interfaceId;
    Ipv6Address m_ifaceAddr; // link-local
    Ipv6Prefix m_ifaceMask;
    uint32_t m_areaId;
    Time m_helloInterval;
    Time m_routerDeadInterval;
    uint32_t m_ifaceTransDelay;
    uint8_t m_routerPriority;
    std::vector<OSPFLSAHeader> m_linkLocalLsas;
    Timer m_helloTimer; // helloIntervalごと
    Timer m_waitTimer; // WaitingになったらrouterDeadInterval後
    std::map<RouterId, NeighborData> m_neighbors;
    RouterId m_designatedRouterId;
    RouterId m_backupDesignatedRouterId;
    uint32_t m_ifaceOutputCost;
    Time m_rxmtInterval;
    uint16_t m_auType;
    uint8_t m_authenticationKey[8];
    std::map<RouterId, Ipv6Address> m_prefixAddrs;
    std::map<RouterId, uint8_t> m_prefixLengthes;

    Callback<void> m_sendHelloCallback;
    Callback<void, RouterId> m_sendDDCallback;
    Callback<void, RouterId> m_sendLSRCallback;
    Callback<void, RouterId> m_rxmtCallback;
    Timer m_rxmtTimer;

    InterfaceData ();
    bool IsKnownNeighbor(RouterId id) const {return (bool)m_neighbors.count(id);}
    void SetInterfaceIndex(uint32_t ifaceIdx) {m_ifaceIdx = ifaceIdx;}
    void SetHelloSender(Callback<void> &cb) {m_sendHelloCallback = cb;}
    void SetDDSender(Callback<void, RouterId> &cb) {m_sendDDCallback = cb;}
    void SetLSRSender(Callback<void, RouterId> &cb) {m_sendLSRCallback = cb;}
    bool ShouleBeAdjacent(RouterId routerId, NeighborData &neighbor);
    void NotifyEvent(InterfaceEvent event);
    void NotifyNeighborEvent(RouterId routerId, RouterId neighborRouterId, NeighborEvent event);
    /** lsRxmtListに入っているLSAHeaderを抜き出し、DDPacketを送信します */
    void SendDatabaseDescription(RouterId routerId);
    /** lsRequestListに入っているLSAHeaderを抜き出し、LSRPacketを送信します */
    void SendLinkStateRequest(RouterId routerId);
    void SendHello();
    void ScheduleSendDatabaseDescription(RouterId routerId);
    void ScheduleSendLinkStateRequest(RouterId routerId);
    void ScheduleSendHello();
    void RxmtWrapper(RouterId routerId);
    void HelloWrapper();
    void CancelRxmt ();
};

}
}

#endif