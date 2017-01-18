#ifndef OSPF_RTABLE_H
#define OSPF_RTABLE_H

#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6-routing-table-entry.h"
#include <vector>

// OSPFv2 11 The Routing Table Structure
// https://tools.ietf.org/html/rfc2328#page-107

// OSPFv3 4.3 The Routing table Structure
// https://tools.ietf.org/html/rfc5340#section-4.3

namespace ns3 {
namespace ospf {
    class RoutingTable : public Object {
    public:
        RoutingTable();
        ~RoutingTable();
        static TypeId GetTypeId () {
            static TypeId tid = TypeId("ns3::ospf::RoutingTable")
                .AddConstructor<RoutingTable>();
            return tid;
        }
        bool AddRoute(Ipv6RoutingTableEntry &entry);
        // bool RemoveRoute(Ipv6Address &dst);
        bool LookupRoute(Ipv6Address &dst, Ipv6RoutingTableEntry &entry);
        // bool Update(Ipv6RoutingTableEntry &entry);
        void Clear() {
            m_entries.clear();
        }
        std::vector<Ipv6RoutingTableEntry>& GetCollection() {
            return m_entries;
        }
        friend std::ostream& operator<< (std::ostream& os, const RoutingTable& table);
    private:
        std::vector<Ipv6RoutingTableEntry> m_entries;
    };
}
}

#endif /* OSPF_RTABLE_H */