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
// bool RoutingTable::RemoveRoute(Ipv6Address &dst) {
//     NS_LOG_FUNCTION(this << dst);

//     NS_LOG_LOGIC("Route deletion failed:" << dst);
//     return false;
// }
bool RoutingTable::AddRoute(Ipv6RoutingTableEntry &entry) {
    NS_LOG_FUNCTION(this << entry.GetDest() << entry.GetDestNetworkPrefix());

    m_entries.push_back(entry);
    return true;
}
// bool RoutingTable::Update(Ipv6RoutingTableEntry &entry) {
//     NS_LOG_FUNCTION(this);

//     NS_LOG_LOGIC("Route update successed");
//     return true;
// }

}
}