#ifndef OSPF_RTABLE_H
#define OSPF_RTABLE_H

#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv6-address.h"
#include <vector>
#include <unordered_map>
#include <tuple>

// OSPFv2 11 The Routing Table Structure
// https://tools.ietf.org/html/rfc2328#page-107

// OSPFv3 4.3 The Routing table Structure
// https://tools.ietf.org/html/rfc5340#section-4.3

namespace ns3 {
namespace ospf {
    typedef uint32_t RouterId;
    class RoutingTable : public Object {
    public:
        RoutingTable();
        ~RoutingTable();
        static TypeId GetTypeId () {
            static TypeId tid = TypeId("ns3::ospf::RoutingTable")
                .AddConstructor<RoutingTable>();
            return tid;
        }
        RouterId LookupRoute(RouterId src, RouterId dst);
        RouterId GetNearestRouter(Ipv6Address& addr, RouterId ignoreId = 0);
        void AddFlow(RouterId src, RouterId dst, RouterId nextHop, uint16_t flowValue);
        void AddRouter(Ipv6Address& addr, Ipv6Prefix& prefix, RouterId routerId);
        void SetRouters(uint32_t routers);
        void Reset();
        void FinalizeFlow();
        friend std::ostream& operator<< (std::ostream& os, const RoutingTable& table);
    private:
        uint32_t m_numOfRouters;
        std::vector<std::vector<std::vector<uint16_t> > > m_nextHop_table; // [src][dst][nextHop] = flow
        std::vector<std::vector<std::vector<float> > > m_nextProb_table; // [src][dst][nextHop] = flow
        std::vector<std::tuple<Ipv6Address, Ipv6Prefix, RouterId> > m_router_tuples; // [RouterId] = addr
    };
}
}

#endif /* OSPF_RTABLE_H */