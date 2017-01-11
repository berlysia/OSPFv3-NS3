#ifndef OSPF_STRUCT_NEIGHBOR_H
#define OSPF_STRUCT_NEIGHBOR_H

#include "ns3/timer.h"
#include "ns3/callback.h"
#include "ns3/ipv6-address.h"
#include "ospf-hello.h"
#include "ospf-database-description.h"
#include "ospf-lsa.h"
#include "ospf-lsa-header.h"
#include "ospf-constants.h"
#include <vector>
#include <map>
#include <algorithm>

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
    std::vector<OSPFLSA> m_lsRxmtList;
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

    RouterId GetRouterId () const {
        return m_routerId;
    }

    void SetState(NeighborState s) {
        m_state = s;
    }
    NeighborState GetState() const {
        return m_state;
    }
    bool IsState(NeighborState s) const {
        return m_state == s;
    }

    uint8_t GetRouterPriority () const {
        return m_routerPriority;
    }

    void SetAsMaster () {
        m_isMaster = true;
    }

    void SetAsSlave () {
        m_isMaster = false;
    }

    bool IsMaster () const {
        return m_isMaster;
    }

    bool IsSlave () const {
        return !m_isMaster;
    }

    RouterId GetDesignatedRouter() const {
        return m_designatedRouterId;
    }

    RouterId GetBackupDesignatedRouter() const {
        return m_backupDesignatedRouterId;
    }

    bool IsExchangeDone () {
        return m_lsdbSummaryList.size() == 0;
    }

    std::vector<OSPFLSA>& GetRxmtList () {
        return m_lsRxmtList;
    }

    void AddRxmtList(OSPFLSA lsa) {
        m_lsRxmtList.push_back(lsa);
    }

    bool HasInRxmtList (OSPFLinkStateIdentifier &id) {
        for (OSPFLSA& item : m_lsRxmtList) {
            // ここにあるLSAは中身が入っているはず
            if (*item.GetHeader() == id) return true;
        }
        return false;
    }

    void RemoveFromRxmtList(OSPFLinkStateIdentifier &id) {
        m_lsRxmtList.erase(std::remove(m_lsRxmtList.begin(), m_lsRxmtList.end(), id), m_lsRxmtList.end());
    }

    std::vector<OSPFLSAHeader>& GetRequestList () {
        return m_lsRequestList;
    }

    void AddRequestList (OSPFLSAHeader header) {
        m_lsRequestList.push_back(header);
    }

    bool HasInRequestList (OSPFLinkStateIdentifier &id) {
        for (const OSPFLSAHeader& item : m_lsRequestList) {
            if (item == id) return true;
        }
        return false;
    }

    OSPFLSAHeader& GetFromRequestList (OSPFLinkStateIdentifier &id) {
        auto itr = std::find(m_lsRequestList.begin(), m_lsRequestList.end(), id);
        return *itr; // 使う前にHasInRequestListすること
    }

    void RemoveFromRequestList(OSPFLinkStateIdentifier &id) {
        m_lsRequestList.erase(std::remove(m_lsRequestList.begin(), m_lsRequestList.end(), id), m_lsRequestList.end());
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

    bool IsRequestListEmpty () const {
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