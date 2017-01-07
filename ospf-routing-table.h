#ifndef OSPF_RTABLE_H
#define OSPF_RTABLE_H

#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6-routing-table-entry.h"
#include <map>

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
        bool RemoveRoute(Ipv6Address &dst);
        bool LookupRoute(Ipv6Address &dst, Ipv6RoutingTableEntry &entry);
        bool Update(Ipv6RoutingTableEntry &entry);
        void Clear() {
            m_table.clear();
        }
        std::map<Ipv6Address, Ipv6RoutingTableEntry> GetTable() {
            return m_table;
        }
    private:
        std::map<Ipv6Address, Ipv6RoutingTableEntry> m_table;
        typedef std::map<Ipv6Address, Ipv6RoutingTableEntry>::iterator RTIterator;
        typedef std::map<Ipv6Address, Ipv6RoutingTableEntry>::const_iterator RTCIterator;
    };
}
}

#endif /* OSPF_RTABLE_H */