#ifndef OSPF_LSDB_H
#define OSPF_LSDB_H

#include "ns3/nstime.h"
#include "ospf-constants.h"
#include "ospf-lsa.h"
#include "ospf-lsa-identifier.h"
#include <set>
#include <map>
#include <algorithm>

namespace ns3 {
namespace ospf {

class OSPFLSDB {
    std::map<OSPFLinkStateIdentifier, Ptr<OSPFLSA>> m_db;
    std::map<OSPFLinkStateIdentifier, Time> m_addedTime;
    std::map<OSPFLinkStateIdentifier, uint16_t> m_addedAge;

public:
    OSPFLSDB() {}
    ~OSPFLSDB() {
        for (auto it = m_db.begin(); it != m_db.end(); it = m_db.erase(it)) {
        }
        for (auto it = m_addedTime.begin(); it != m_addedTime.end(); it = m_addedTime.erase(it)) {
        }
        for (auto it = m_addedAge.begin(); it != m_addedAge.end(); it = m_addedAge.erase(it)) {
        }
    }

    std::map<OSPFLinkStateIdentifier, Ptr<OSPFLSA>>& GetTable () {
        return m_db;
    }

    void Add(Ptr<OSPFLSA> lsa) {
        OSPFLinkStateIdentifier id = lsa->GetIdentifier();
        m_db[id] = lsa;
        m_addedTime[id] = ns3::Now();
        m_addedAge[id] = lsa->GetHeader()->GetAge();
    }

    bool DetectMaxAge(OSPFLinkStateIdentifier id) {
        return (ns3::Now() - m_addedTime[id]).ToInteger(Time::S) + m_addedAge[id] >= g_maxAge;
    }

    bool Has(OSPFLinkStateIdentifier id) {
        return m_db.count(id);
    }

    uint32_t CalcAge(const OSPFLinkStateIdentifier& id) {
        return (uint32_t)(ns3::Now() - m_addedTime[id]).ToInteger(Time::S) + m_addedAge[id];
    }

    Ptr<OSPFLSA> Get(const OSPFLinkStateIdentifier& id) {
        m_db[id]->GetHeader()->SetAge(std::min(CalcAge(id), g_maxAge));
        return m_db[id];
    }

    void Remove(OSPFLinkStateIdentifier id) {
        m_db.erase(id);
    }

    bool IsElapsedMinLsArrival(OSPFLinkStateIdentifier id) {
        return m_addedTime[id] + g_minLsArrival <= Now();
    }

    std::vector<Ptr<OSPFLSA> > Aggregate(std::set<OSPFLinkStateIdentifier> id_set) {
        std::vector<Ptr<OSPFLSA> > ret;
        for (const OSPFLinkStateIdentifier& id : id_set) {
            ret.push_back(Get(id));
        }
        return ret;
    }

    void GetSummary (std::vector<Ptr<OSPFLSAHeader> >& summary, std::vector<Ptr<OSPFLSA> >& rxmt) {
        for (auto& kv : m_db) {
            if (kv.second->GetHeader()->IsASScope()) continue;
            summary.push_back(kv.second->GetHeader());
            if (DetectMaxAge(kv.first)) {
                rxmt.push_back(kv.second);
            }
        }
    }
};

}
}

#endif