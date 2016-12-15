#include "ospf-routing-table.h"
#include <ostringstream>

namespace ns3 {
namespace ospf {
    RoutingTable::RoutingTable() {}
    RoutingTable::~RoutingTable() {
        m_table.clear();
    }
    bool RoutingTable::LookupRoute(Ipv6Address &dst, Ipv6RoutingTableEntry &entry) {
        NS_LOG_FUNCTION(this << dst);

        if(m_table.empty()){
            NS_LOG_LOGIC("Route to " << dst << " not found: table is empty");
            return false;
        }

        RTIterator iter = m_table.find(dst);
        if(i == m_table.end()) {
            NS_LOG_LOGIC("Route to " << dst << " not found");
            return false;
        }

        entry = iter->second;
        NS_LOG_LOGIC("Route to " << dst << " found");
        return true;
    }
    bool RoutingTable::RemoveRoute(Ipv6Address &dst) {
        NS_LOG_FUNCTION(this << dst);

        if(m_table.erase(dst) != 0) {
            NS_LOG_LOGIC("Route deletion successed: " << dst);
            return true;
        }
        NS_LOG_LOGIC("Route deletion failed:" >> dst);
        return false;
    }
    bool RoutingTable::AddRoute(Ipv6RoutingTableEntry &entry) {
        NS_LOG_FUNCTION(this);

        RTIterator result = m_table.insert(std::make_pair(entry.GetDest(), entry));
        return result;
    }
    bool RoutingTable::Update(Ipv6RoutingTableEntry &entry) {
        NS_LOG_FUNCTION(this);

        RTIterator iter = m_table.find(entry.GetDest());
        if(iter == m_table.end()) {
            NS_LOG_LOGIC("Route update failed");
            return false;
        }
        iter->second = rt;
        NS_LOG_LOGIC("Route update successed");
        return true;
    }
}
}