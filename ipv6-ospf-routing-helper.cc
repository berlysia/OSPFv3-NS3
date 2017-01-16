#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/names.h"
#include "ns3/node.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-list-routing.h"
#include "ns3/assert.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6-routing-protocol.h"

#include "ipv6-ospf-routing-helper.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE ("Ipv6OspfRoutingHelper");

Ipv6OspfRoutingHelper::Ipv6OspfRoutingHelper ()
{
}

Ipv6OspfRoutingHelper::Ipv6OspfRoutingHelper (const Ipv6OspfRoutingHelper &o)
{
}

Ipv6OspfRoutingHelper*
Ipv6OspfRoutingHelper::Copy (void) const
{
  return new Ipv6OspfRoutingHelper (*this);
}

Ptr<Ipv6RoutingProtocol>
Ipv6OspfRoutingHelper::Create (Ptr<Node> node) const
{
  return CreateObject<Ipv6OspfRouting> ();
}

Ptr<Ipv6OspfRouting>
Ipv6OspfRoutingHelper::GetOspfRouting (Ptr<Ipv6> ipv6) const
{
  NS_LOG_FUNCTION (this);
  Ptr<Ipv6RoutingProtocol> ipv6rp = ipv6->GetRoutingProtocol ();
  NS_ASSERT_MSG (ipv6rp, "No routing protocol associated with Ipv6");
  if (DynamicCast<Ipv6OspfRouting> (ipv6rp))
    {
      NS_LOG_LOGIC ("Ospf routing found as the main IPv4 routing protocol.");
      return DynamicCast<Ipv6OspfRouting> (ipv6rp); 
    } 
  if (DynamicCast<Ipv6ListRouting> (ipv6rp))
    {
      Ptr<Ipv6ListRouting> lrp = DynamicCast<Ipv6ListRouting> (ipv6rp);
      int16_t priority;
      for (uint32_t i = 0; i < lrp->GetNRoutingProtocols ();  i++)
        {
          NS_LOG_LOGIC ("Searching for static routing in list");
          Ptr<Ipv6RoutingProtocol> temp = lrp->GetRoutingProtocol (i, priority);
          if (DynamicCast<Ipv6OspfRouting> (temp))
            {
              NS_LOG_LOGIC ("Found static routing in list");
              return DynamicCast<Ipv6OspfRouting> (temp);
            }
        }
    }
  NS_LOG_LOGIC ("Ospf routing not found");
  return 0;
}

}
}
