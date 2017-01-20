#include "ospf-routing-table.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE ("RoutingTable");

NS_OBJECT_ENSURE_REGISTERED (RoutingTable);

RoutingTable::RoutingTable() {}
RoutingTable::~RoutingTable() {
    m_entries.clear();
}
bool RoutingTable::LookupRoute(Ipv6Address &dst, Ipv6RoutingTableEntry &ret) {
    NS_LOG_FUNCTION(this << dst);

    if(m_entries.empty()){
        NS_LOG_LOGIC("Route to " << dst << " not found: table is empty");
        return false;
    }

    for (auto& entry : m_entries) {
        if (entry.GetDestNetworkPrefix().IsMatch(dst, entry.GetDest())) {
            ret = entry;
            return true;
        }
    }

    NS_LOG_LOGIC("Route to " << dst << " not found");
    return false;
}

bool RoutingTable::AddRoute(Ipv6RoutingTableEntry &entry) {
    NS_LOG_FUNCTION(this << entry.GetDest() << entry.GetDestNetworkPrefix());

    m_entries.push_back(entry);
    return true;
}

std::ostream& operator<< (std::ostream& os, const RoutingTable& table) {
    for (auto& entry : table.m_entries) {
        os << entry << "\n";
    }
    return os;
}

}
}