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
typedef NeighborStateNS::Type NeighborState;

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
typedef NeighborEventNS::Type NeighborEvent;

std::string ToString ( const NeighborState& value );
std::string ToString ( const NeighborEvent& value );

class NeighborData {
    typedef uint32_t RouterId;
    NeighborState m_state;
    Timer m_inactivityTimer; // 初期値はrouterDeadInterval, HelloPacket受信でリセット
    Timer m_lastReceivedDdClearTimer; // 初期値はrouterDeadInterval, HelloPacket受信でリセット
    bool m_isMaster; // ExStart時に決定
    int32_t m_ddSeqNum;
    OSPFDatabaseDescription* m_lastReceivedDd;
    uint32_t m_routerId;
    uint8_t m_routerPriority;
    uint32_t m_routerIfaceId;
    Ipv6Address m_addr;
    RouterId m_designatedRouterId;
    RouterId m_backupDesignatedRouterId;
    std::vector<Ptr<OSPFLSA> > m_lsRxmtList;
    std::vector<Ptr<OSPFLSAHeader> > m_lsRequestList;
    std::vector<Ptr<OSPFLSAHeader> > m_lsdbSummaryList;

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

    uint32_t GetInterfaceId() const {
        return m_routerIfaceId;
    }

    void SetLastReceivedDD(OSPFDatabaseDescription dd) {
        dd.GetLSAHeaders().clear(); // コピーされているので問題ないはず
        m_lastReceivedDd = new OSPFDatabaseDescription(dd);
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
        return *m_lastReceivedDd;
    }

    void ClearLastPacket() {
        m_lastReceivedDd = nullptr;
    }

    Timer& GetInactivityTimer () {
        return m_inactivityTimer;
    }

    Timer& GetLastPacketClearTimer () {
        return m_lastReceivedDdClearTimer;
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

    Ipv6Address& GetAddress () {
        return m_addr;
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

    std::vector<Ptr<OSPFLSA> >& GetRxmtList () {
        return m_lsRxmtList;
    }

    std::vector<Ptr<OSPFLSA> > GetRxmtList (uint32_t maxBytes) {
        std::vector<Ptr<OSPFLSA> > ret;
        for (int i = 0, l = m_lsRxmtList.size(); i < l; ++i) {
            maxBytes -= m_lsRxmtList[i]->GetSerializedSize();
            if (maxBytes <= 0) break;
            ret.push_back(m_lsRxmtList[i]);
        }
        return ret;
    }

    void AddRxmtList(Ptr<OSPFLSA> lsa) {
        m_lsRxmtList.push_back(lsa);
    }

    bool HasInRxmtList (OSPFLinkStateIdentifier &id) {
        for (auto item : m_lsRxmtList) {
            // ここにあるLSAは中身が入っているはず
            if (*item->GetHeader() == id) {
                return true;
            }
        }
        return false;
    }

    Ptr<OSPFLSA> GetFromRxmtList (OSPFLinkStateIdentifier &id) {
        for (auto item : m_lsRxmtList) {
            // ここにあるLSAは中身が入っているはず
            if (*item->GetHeader() == id) {
                return item;
            }
        }
        return 0;
    }

    void RemoveFromRxmtList(OSPFLinkStateIdentifier id) {
        for (auto itr = m_lsRxmtList.begin(); itr != m_lsRxmtList.end(); ) {
            if (**itr == id) {
                itr = m_lsRxmtList.erase(itr);
            } else {
                itr++;
            }
        }
    }

    std::vector<Ptr<OSPFLSAHeader> >& GetRequestList () {
        return m_lsRequestList;
    }

    void AddRequestList (Ptr<OSPFLSAHeader> header) {
        m_lsRequestList.push_back(header);
    }

    bool HasInRequestList (OSPFLinkStateIdentifier &id) {
        for (auto item : m_lsRequestList) {
            if (*item == id) return true;
        }
        return false;
    }

    Ptr<OSPFLSAHeader> GetFromRequestList (OSPFLinkStateIdentifier &id) {
        for (auto item : m_lsRequestList) {
            if (*item == id) {
                return item;
            }
        }
        return 0;
    }

    void RemoveFromRequestList(OSPFLinkStateIdentifier &id) {
        for (auto itr = m_lsRequestList.begin(); itr != m_lsRequestList.end(); ) {
            if (**itr == id) {
                itr = m_lsRequestList.erase(itr);
            } else {
                itr++;
            }
        }
    }

    std::vector<Ptr<OSPFLSAHeader> > GetRequestList (uint32_t maxBytes) {
        std::vector<Ptr<OSPFLSAHeader> > ret;
        for (int i = 0, l = std::min(m_lsRequestList.size(), (unsigned long)(maxBytes / 20)); i < l; ++i) {
            ret.push_back(m_lsRequestList[i]);
        }
        return ret;
    }

    void SetSummaryList (std::vector<Ptr<OSPFLSAHeader> > summary) {
        m_lsdbSummaryList = summary;
    }

    void RemoveFromSummaryList(OSPFLinkStateIdentifier &id) {
        for (auto itr = m_lsdbSummaryList.begin(); itr != m_lsdbSummaryList.end(); ) {
            if (**itr == id) {
                itr = m_lsdbSummaryList.erase(itr);
            } else {
                itr++;
            }
        }
    }

    uint32_t GetSummaryListSize () {
        return m_lsdbSummaryList.size();
    }
    std::vector<Ptr<OSPFLSAHeader> > GetSummaryList (uint32_t maxBytes) {
        std::vector<Ptr<OSPFLSAHeader> > ret;
        for (int i = maxBytes / 20; i && !m_lsdbSummaryList.empty(); --i) {
            ret.push_back(m_lsdbSummaryList.back());
            m_lsdbSummaryList.pop_back();
        }
        return ret;
    }

    bool HasMoreSummary () const {
        return m_lsdbSummaryList.size();
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