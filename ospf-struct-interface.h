#ifndef OSPF_STRUCT_INTERFACE_H
#define OSPF_STRUCT_INTERFACE_H

#include "ns3/timer.h"
#include "ns3/callback.h"
#include "ospf-struct-neighbor.h"
#include "ospf-lsa-header.h"
#include "ospf-lsa.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>

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

std::string ToString (const InterfaceType& type );
std::string ToString (const InterfaceState& type );
std::string ToString (const InterfaceEvent& type );
    
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
    std::set<OSPFLinkStateIdentifier> m_linkLocalLsa_set;
    Timer m_helloTimer; // helloIntervalごと
    Timer m_waitTimer; // WaitingになったらrouterDeadInterval後
    std::map<RouterId, NeighborData> m_neighbors;
    RouterId m_designatedRouterId;
    RouterId m_backupDesignatedRouterId;
    uint32_t m_ifaceOutputCost;
    Time m_rxmtInterval;
    uint16_t m_auType;
    uint8_t m_authenticationKey[8];
    std::map<RouterId, std::vector<Ipv6Address> > m_prefixAddrs;
    std::map<RouterId, std::vector<uint8_t> > m_prefixLengthes;

public:
    InterfaceData () {
        // m_type;
        m_state = InterfaceState::DOWN;
        // m_interfaceId;
        // m_ifaceAddr;
        // m_ifaceMask;
        m_areaId = 1;
        m_helloInterval = Seconds(10.0);
        m_routerDeadInterval = Seconds(40.0);
        m_ifaceTransDelay = 1;
        // m_routerPriority;
        m_designatedRouterId = 0;
        m_backupDesignatedRouterId = 0;
        m_ifaceOutputCost = 1;
        m_rxmtInterval = Seconds(5.0);
        m_auType = 0;
        m_helloTimer = Timer(Timer::REMOVE_ON_DESTROY);
        m_helloTimer.SetDelay(m_helloInterval);
        m_waitTimer = Timer(Timer::REMOVE_ON_DESTROY);
        m_waitTimer.SetDelay(m_routerDeadInterval);
        std::fill(m_authenticationKey, m_authenticationKey+8, 0);
    }

    void ResetInstance () {
        m_areaId = 1;
        m_helloInterval = Seconds(10.0);
        m_routerDeadInterval = Seconds(40.0);
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
    bool IsType(InterfaceType type) const {
        return m_type == type;
    }

    InterfaceState GetState () {
        return m_state;
    }

    void SetState(InterfaceState state) {
        m_state = state;
    }
    bool IsState(InterfaceState state) const {
        return m_state == state;
    }

    Ipv6Address& GetAddress () {
        return m_ifaceAddr;
    }

    void SetAddress (Ipv6Address addr) {
        m_ifaceAddr = addr;
    }

    Ipv6Prefix& GetPrefix () {
        return m_ifaceMask;
    }

    void SetPrefix (Ipv6Prefix prefix) {
        m_ifaceMask = prefix;
    }

    uint32_t GetAreaId () const {
        return m_areaId;
    }

    Time& GetHelloInterval () {
        return m_helloInterval;
    }

    Time& GetRouterDeadInterval () {
        return m_routerDeadInterval;
    }

    Time& GetRxmtInterval () {
        return m_rxmtInterval;
    }

    bool IsIndex(uint32_t ifaceIdx) const {
        return m_interfaceId == ifaceIdx;
    }

    uint32_t GetIfaceTransDelay () const {
        return m_ifaceTransDelay;
    }

    uint8_t GetRouterPriority () const {
        return m_routerPriority;
    }

    void AddLinkLocalLSA(RouterId routerId, Ptr<OSPFLSA> lsa) {
        m_linkLocalLsa_set.insert(lsa->GetIdentifier());
        auto lsBody = lsa->GetBody<OSPFLinkLSABody>();
        ClearPrefix(routerId);
        for (int i = 0, l = lsBody->CountPrefixes(); i < l; ++i) {
            AddPrefix(routerId, lsBody->GetPrefixAddress(i), lsBody->GetPrefixLength(i));
        }
    }

    bool IsKnownLinkLocalLSA(OSPFLinkStateIdentifier id) {
        return m_linkLocalLsa_set.count(id);
    }

    std::set<OSPFLinkStateIdentifier> GetLinkLocalLSAIds () {
        return m_linkLocalLsa_set;
    }

    bool IsEligibleToDR () const {
        return m_routerPriority > 0;
    }

    void SetInterfaceId(uint32_t ifaceIdx) {
        m_interfaceId = ifaceIdx;
    }
    uint32_t GetInterfaceId () {
        return m_interfaceId;
    }
    NeighborData& GetNeighbor(RouterId routerId) {
        return m_neighbors[routerId];
    }
    bool HasNeighbor(RouterId routerId) const {
        return m_neighbors.count(routerId);
    }
    std::map<RouterId, NeighborData>& GetNeighbors() {
        return m_neighbors;
    }
    uint32_t CountNeighbors () const {
        return m_neighbors.size();
    }
    uint32_t CountActiveNeighbors () const {
        uint32_t ret = 0;
        for (auto& kv : m_neighbors) {
            if (kv.second.GetState() >= NeighborState::TWOWAY) {
                ret++;
            }
        }
        return ret;
    }
    std::vector<RouterId> GetActiveNeighbors() const {
        std::vector<RouterId> neighs;
        for (auto& kv : m_neighbors) {
            if (kv.second.GetState() >= NeighborState::TWOWAY) {
                neighs.push_back(kv.first);
            }
        }
        return neighs;
    }
    bool IsKnownNeighbor(RouterId id) const {
        return (bool)m_neighbors.count(id);
    }
    InterfaceType GetType() const {
        return m_type;
    }

    RouterId GetDesignatedRouter() const {
        return m_designatedRouterId;
    }

    RouterId GetBackupDesignatedRouter() const {
        return m_backupDesignatedRouterId;
    }

    bool HasExchangingNeighbor () const {
        for (auto& kv : m_neighbors) {
            if (
                kv.second.IsState(NeighborState::EXCHANGE) ||
                kv.second.IsState(NeighborState::LOADING)
            ) {
                return true;
            }
        }
        return false;
    }

    void AddRxmtList(Ptr<OSPFLSA> lsa) {
        for (auto& kv : m_neighbors) {
            kv.second.AddRxmtList(lsa);
        }
    }

    void ClearPrefix(RouterId routerId) {
        m_prefixAddrs[routerId].clear();
        m_prefixLengthes[routerId].clear();
    }

    void AddPrefix(RouterId routerId, Ipv6Address addr, uint8_t length) {
        m_prefixAddrs[routerId].push_back(addr);
        m_prefixLengthes[routerId].push_back(length);
    }

    uint32_t CountPrefixes (RouterId routerId) {
        return m_prefixAddrs[routerId].size();
    }

    Ipv6Address& GetPrefixAddress(RouterId routerId, uint32_t idx) {
        return m_prefixAddrs[routerId][idx];
    }

    uint8_t GetPrefixLength(RouterId routerId, uint32_t idx) {
        return m_prefixLengthes[routerId][idx];
    }

    Timer& GetWaitTimer () {
        return m_waitTimer;
    }

    void StartWaitTimer () {
        m_waitTimer.Schedule();
    }

    Timer& GetHelloTimer () {
        return m_helloTimer;
    }

    void ScheduleHello () {
        m_helloTimer.Schedule();
    }

    bool IsActive () {
        return !(
            IsState(InterfaceState::DOWN) ||
            IsState(InterfaceState::LOOPBACK)
        );
    }

    void CalcDesignatedRouter() {
        // OSPFv2 9.4 Electioning the Designated Router
        // https://tools.ietf.org/html/rfc2328#page-75

        // XXX: 今の所いらないので使うときに書いてください
        // 結果によってDR, BDR, DR_OTHERをSetStateして抜けてください
    }
};

}
}

#endif