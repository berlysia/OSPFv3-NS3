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
    P2P,
    BROADCAST,
    NBMA,
    P2M,
    VIRTUAL,
};
}
typedef InterfaceTypeNS::Type InterfaceType;

namespace InterfaceStateNS {
enum Type {
    DOWN,
    P2P,
    LOOPBACK,
    WAITING,
    DR_OTHER,
    BACKUP,
    DR,
};
}
typedef InterfaceStateNS::Type InterfaceState;

namespace InterfaceEventNS {
enum Type {
    IF_UP,
    WAIT_TIMER,
    BACKUP_SEEN,
    NEIGH_CHANGE,
    LOOP_IND,
    UNLOOP_IND,
    IF_DOWN,
};
}
typedef InterfaceEventNS::Type InterfaceEvent;
    
class InterfaceData {
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

public:
    InterfaceData () {
        m_state = InterfaceState::DOWN;
        m_areaId = 0;
        m_helloInterval = Seconds(10.0);
        m_routerDeadInterval = Seconds(10.0);
        m_ifaceTransDelay = 1;
        m_designatedRouterId = 0;
        m_backupDesignatedRouterId = 0;
        m_ifaceOutputCost = 1;
        m_rxmtInterval = Seconds(5.0);
        m_auType = 0;
        m_waitTimer.SetDelay(m_routerDeadInterval);
    }

    void ResetInstance () {
        m_areaId = 0;
        m_helloInterval = Seconds(10.0);
        m_routerDeadInterval = Seconds(10.0);
        m_ifaceTransDelay = 1;
        m_designatedRouterId = 0;
        m_backupDesignatedRouterId = 0;
        m_ifaceOutputCost = 1;
        m_rxmtInterval = Seconds(5.0);
        m_auType = 0;
        m_helloTimer.Cancel();
        m_waitTimer.Cancel();
    }

    void SetType(InterfaceType type) {
        m_type = type;
    }
    bool IsType(InterfaceType type) {
        return m_type == type;
    }

    void SetState(InterfaceState state) {
        m_state = state;
    }
    bool IsState(InterfaceState state) {
        return m_state == state;
    }

    uint8_t GetRouterPriority () {
        return m_routerPriority;
    }

    bool IsEligibleToDR () {
        return m_routerPriority > 0;
    }

    void SetInterfaceId(uint32_t ifaceId) {
        m_interfaceId = ifaceId;
    }
    NeighborData& GetNeighbor(RouterId routerId) {
        return m_neighbors[routerId];
    }
    std::map<RouterId, NeighborData>& GetNeighbors() {
        return m_neighbors;
    }
    bool IsKnownNeighbor(RouterId id) const {
        return (bool)m_neighbors.count(id);
    }
    InterfaceType GetType() {
        return m_type;
    }

    RouterId GetDesignatedRouter() {
        return m_designatedRouterId;
    }

    RouterId GetBackupDesignatedRouter() {
        return m_backupDesignatedRouterId;
    }

    Timer& GetWaitTimer () {
        return m_waitTimer;
    }

    void StartWaitTimer () {
        m_waitTimer.Schedule();
    }

    void SetHelloSender(Callback<void, uint32_t, RouterId> cb) {}

    void CalcDesignatedRouter() {
        // OSPFv2 9.4 Electiong the Designated Router
        // https://tools.ietf.org/html/rfc2328#page-75

        // XXX: 今の所いらないので使うときに書いてください
        // 結果によってDR, BDR, DR_OTHERをSetStateして抜けてください
    }
};

}
}

#endif