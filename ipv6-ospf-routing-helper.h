#ifndef IPV6_OSPFV3_ROUTING_HELPER_H
#define IPV6_OSPFV3_ROUTING_HELPER_H

#include "ns3/ipv6.h"
#include "ns3/ptr.h"
#include "ns3/ipv6-address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/ipv6-routing-helper.h"
#include "ipv6-ospf-routing.h"

namespace ns3 {
namespace ospf {

class Ipv6OspfRoutingHelper : public Ipv6RoutingHelper
{
public:
  Ipv6OspfRoutingHelper ();

  Ipv6OspfRoutingHelper (const Ipv6OspfRoutingHelper &);

  Ipv6OspfRoutingHelper* Copy (void) const;

  virtual Ptr<Ipv6RoutingProtocol> Create (Ptr<Node> node) const;

  Ptr<Ipv6OspfRouting> GetOspfRouting (Ptr<Ipv6> ipv6) const;
};

}
}

#endif