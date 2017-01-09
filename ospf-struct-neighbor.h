#ifndef OSPF_STRUCT_NEIGHBOR_H
#define OSPF_STRUCT_NEIGHBOR_H

#include "ns3/timer.h"
#include "ns3/callback.h"
#include "ns3/ipv6-address.h"
#include "ospf-hello.h"
#include "ospf-database-description.h"
#include "ospf-lsa-header.h"
#include "ospf-constants.h"
#include <vector>
#include <map>

namespace ns3 {
namespace ospf {

namespace NeighborStateNS {
enum Type {
    DOWN,
    ATTEMPT,
    INIT,
    TWOWAY,
    EXSTART,
    EXCHANGE,
    LOADING,
    FULL,
};
}

namespace NeighborEventNS {
enum Type {
    HELLO_RECEIVED,
    START,
    TWOWAY_RECEIVED,
    NEGOT_DONE,
    EXCHANGE_DONE,
    BAD_LS_REQ,
    LOADING_DONE,
    IS_ADJ_OK,
    SEQ_NUM_MISMATCH,
    ONEWAY_RECEIVED,
    KILL_NBR,
    INACTIVE,
    LL_DOWN,
};
}

typedef NeighborStateNS::Type NeighborState;
typedef NeighborEventNS::Type NeighborEvent;

class NeighborData {
    typedef uint32_t RouterId;
    NeighborState m_state;
    Timer m_inactivityTimer; // 初期値はrouterDeadInterval, HelloPacket受信でリセット
    bool m_isMaster; // ExStart時に決定
    int32_t m_ddSeqNum;
    OSPFDatabaseDescription m_lastReceivedDd;
    uint32_t m_routerId;
    uint8_t m_routerPriority;
    uint32_t m_routerIfaceId;
    Ipv6Address m_addr;
    RouterId m_designatedRouterId;
    RouterId m_backupDesignatedRouterId;
    std::vector<OSPFLSAHeader> m_lsRxmtList;
    std::vector<OSPFLSAHeader> m_lsRequestList;
    std::vector<OSPFLSAHeader> m_lsdbSummaryList;

    bool m_initialized;

public:
    NeighborData () {
        m_state = NeighborState::DOWN;
        m_ddSeqNum = 0;

        m_designatedRouterId = 0;
        m_backupDesignatedRouterId = 0;

        m_initialized = false;
    }
    ~NeighborData () {
        m_lsRxmtList.clear();
        m_lsRequestList.clear();
        m_lsdbSummaryList.clear();
    }

    bool IsInitialized () {
        return m_initialized;
    }

    void MinimalInitialize (Ipv6Address &addr, OSPFHello &hello) {
        m_routerId = hello.GetRouterId();
        m_routerIfaceId = hello.GetInterfaceId();
        m_addr = addr;
    }

    void Initialize (Ipv6Address &addr, OSPFHello &hello) {
        m_routerId = hello.GetRouterId();
        m_routerPriority = hello.GetRouterPriority();
        m_routerIfaceId = hello.GetInterfaceId();
        m_addr = addr;
        m_designatedRouterId = hello.GetDesignatedRouter();
        m_backupDesignatedRouterId = hello.GetBackupDesignatedRouter();
        m_initialized = true;
    }

    void SetLastReceivedDD(OSPFDatabaseDescription &dd) {
        dd.GetLSAHeaders().clear();
        m_lastReceivedDd = dd;
    }

    void SetSequenceNumber(int32_t seqNum) {
        m_ddSeqNum = seqNum;
    }

    int32_t GetSequenceNumber() {
        return m_ddSeqNum;
    }

    void IncrementSequenceNumber () {
        m_ddSeqNum++;
    }

    OSPFDatabaseDescription& GetLastPacket() {
        return m_lastReceivedDd;
    }

    Timer& GetInactivityTimer () {
        return m_inactivityTimer;
    }

    RouterId GetRouterId () {
        return m_routerId;
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

    uint8_t GetRouterPriority () {
        return m_routerPriority;
    }

    void SetAsMaster () {
        m_isMaster = true;
    }

    void SetAsSlave () {
        m_isMaster = false;
    }

    bool IsMaster () {
        return m_isMaster;
    }

    bool IsSlave () {
        return !m_isMaster;
    }

    RouterId GetDesignatedRouter() {
        return m_designatedRouterId;
    }

    RouterId GetBackupDesignatedRouter() {
        return m_backupDesignatedRouterId;
    }

    void AddLinkStateRequestList (OSPFLSAHeader header) {
        m_lsRequestList.push_back(header);
    }

    bool IsExchangeDone () {
        return m_lsdbSummaryList.size() == 0;
    }

    std::vector<OSPFLSAHeader>& GetRxmtList () {
        return m_lsRxmtList;
    }

    bool IsEligibleToDR () const {
        return m_routerPriority > 0;
    }

    bool IsDR () const {
        return m_routerId == m_designatedRouterId;
    }

    bool IsBDR () const {
        return m_routerId == m_backupDesignatedRouterId;
    }

    bool IsRequestListEmpty () {
        return m_lsRequestList.empty();
    }

    void StartExchange() {
        m_ddSeqNum++;
        m_isMaster = true;
    }

    void ClearList() {
        m_lsRxmtList.clear();
        m_lsRequestList.clear();
        m_lsdbSummaryList.clear();
    }
};

}
}

#endif