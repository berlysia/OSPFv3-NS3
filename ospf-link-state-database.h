#ifndef OSPF_LSDB_H
#define OSPF_LSDB_H

#include "ns3/nstime.h"
#include "ospf-constants.h"
#include "ospf-lsa.h"
#include "ospf-lsa-identifier.h"
#include <set>
#include <map>

namespace ns3 {
namespace ospf {

class OSPFLSDB {
    std::map<OSPFLinkStateIdentifier, OSPFLSA> m_db;
    std::map<OSPFLinkStateIdentifier, Time> m_added;

public:
    void Add(OSPFLSA lsa) {
        OSPFLinkStateIdentifier id = lsa.GetIdentifier();
        m_db[id] = lsa;
        m_added[id] = ns3::Now();
    }

    bool Has(OSPFLinkStateIdentifier id) {
        return m_db.count(id);
    }

    OSPFLSA& Get(OSPFLinkStateIdentifier id) {
        return m_db[id];
    }

    bool IsElapsedMinLsArrival(OSPFLinkStateIdentifier id) {
        return m_added[id] + g_minLsArrival <= Now();
    }
};

}
}

#endif