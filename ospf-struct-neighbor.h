#ifndef OSPF_STRUCT_NEIGHBOR_H
#define OSPF_STRUCT_NEIGHBOR_H

#include "ns3/timer.h"
#include "ns3/callback.h"
#include "ns3/ipv6-address.h"
#include "ospf-database-description.h"
#include "ospf-lsa-header.h"
#include "ospf-constants.h"
#include <vector>
#include <map>

namespace ns3 {
namespace ospf {

namespace NeighborStateNS {
enum Type {
    DOWN = 1,
    ATTEMPT = 2,
    INIT = 3,
    TWOWAY = 4,
    EXSTART = 5,
    EXCHANGE = 6,
    LOADING = 7,
    FULL = 8,
};
}

namespace NeighborEventNS {
enum Type {
    HELLORECEIVED = 1,
    START = 2,
    TWOWAYRECEIVED = 3,
    NEGOTIATIONDONE = 4,
    EXCHANGEDONE = 5,
    BADLSREQ = 6,
    LOADINGDONE = 7,
    ISADJOK = 8,
    SEQNUMMISMATCH = 9,
    ONEWAYRECEIVED = 10,
    KILLNBR = 11,
    INACTIVE = 12,
    LLDOWN = 13,
};
}

typedef NeighborStateNS::Type NeighborState;
typedef NeighborEventNS::Type NeighborEvent;

struct NeighborData {
    typedef uint32_t RouterId;
    NeighborState m_state;
    Timer m_inactivityTimer; // 初期値はrouterDeadInterval, HelloPacket受信でリセット
    bool m_isMaster; // ExStart時に決定
    int32_t m_sendingDdSeqNum;
    OSPFDatabaseDescription m_lastReceivedDd;
    uint32_t m_routerId;
    uint32_t m_routerPriority;
    uint32_t m_routerIfaceId;
    Ipv6Address m_addr;
    RouterId m_designatedRouterId;
    RouterId m_backupDesignatedRouterId;
    std::vector<OSPFLSAHeader> m_lsRxmtList;
    std::vector<OSPFLSAHeader> m_lsRequestList;
    std::vector<OSPFLSAHeader> m_lsdbSummaryList;

    NeighborData () {
        m_state = NeighborState::DOWN;
        m_sendingDdSeqNum = g_initialSeqNum;
    }

    void SetState(NeighborState s) {
        m_state = s;
    }
    NeighborState GetState() {
        return m_state;
    }
    bool IsState(NeighborState s) {
        return m_state == s;
    }

    bool IsRequestListEmpty () {
        return m_lsRequestList.empty();
    }

    void StartExchange() {
        m_sendingDdSeqNum++;
        m_isMaster = true;
    }

    void ClearList() {
        m_lsRxmtList.clear();
        m_lsRequestList.clear();
        m_lsdbSummaryList.clear();
    }

    void ResetInactivityTimer (Time &delay) {
        m_inactivityTimer.Cancel();
        m_inactivityTimer.Schedule(delay);
    }

    void SetInactivityTimerArguments (RouterId routerId, NeighborEvent evt) {
        m_inactivityTimer.SetArguments(routerId, evt);
    }

    template<typename MEM_FN_PTR, typename OBJ_PTR>
    void SetInactivityTimerCallback (MEM_FN_PTR cb, OBJ_PTR ob) {
        m_inactivityTimer.SetFunction(cb, ob);
    }

};

}
}

#endif