#ifndef IPV6_OSPFV3_ROUTING_H
#define IPV6_OSPFV3_ROUTING_H

#include <stdint.h>

#include <list>

#include "ns3/ptr.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ospf-routing-table.h"
#include "ospf-struct-interface.h"

namespace ns3 {
namespace ospf {

// class Packet;
// class NetDevice;
// class Ipv6Interface;
// class Ipv6Route;
// class Node;
// class Ipv6RoutingTableEntry;
// class Ipv6MulticastRoutingTableEntry;

class Ipv6OspfRouting : public Ipv6RoutingProtocol
{
public:
  static TypeId GetTypeId ();

  Ipv6OspfRouting ();
  virtual ~Ipv6OspfRouting ();

  virtual Ptr<Ipv6Route> RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);

  virtual bool RouteInput  (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
                            UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                            LocalDeliverCallback lcb, ErrorCallback ecb);

  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address);
  virtual void NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse = Ipv6Address::GetZero ());

  virtual void NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse = Ipv6Address::GetZero ());

  virtual void SetIpv6 (Ptr<Ipv6> ipv6);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

  virtual void HandleProtocolMessage (Ptr<Socket> socket);

  virtual void ReceiveHelloPacket(uint32_t ifaceIdx, Ipv6Address srcAddr, Ptr<Packet> packet);
  virtual void ReceiveDatabaseDescriptionPacket(uint32_t ifaceIdx, Ipv6Address srcAddr, Ptr<Packet> packet);
  virtual void ReceiveLinkStateRequestPacket(uint32_t ifaceIdx, Ipv6Address srcAddr, Ptr<Packet> packet);
  virtual void ReceiveLinkStateUpdatePacket(uint32_t ifaceIdx, Ipv6Address srcAddr, Ptr<Packet> packet);
  virtual void ReceiveLinkStateAckPacket(uint32_t ifaceIdx, Ipv6Address srcAddr, Ptr<Packet> packet);
  
  virtual void Start ();

protected:
  virtual void DoDispose ();

private:
  static const uint32_t PROTO_PORT = 7345;
  static uint32_t ROUTER_ID_SEED;

  uint32_t ROUTER_ID;

  typedef std::map< Ptr<Socket>, uint32_t > SocketToIfaceIdx;

  SocketToIfaceIdx m_socketToIfaceIdx;
  std::vector<InterfaceData> m_interfaces;
  RoutingTable m_routingTable;


  /**
   * \brief Ipv6 reference.
   */
  Ptr<Ipv6> m_ipv6;
};

uint32_t Ipv6OspfRouting::ROUTER_ID_SEED = 1;

} /* namespace ospf */
} /* namespace ns3 */

#endif /* IPV6_STATIC_ROUTING_H */

