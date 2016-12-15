#ifndef IPV6_OSPFV3_ROUTING_H
#define IPV6_OSPFV3_ROUTING_H

#include <stdint.h>

#include <list>

#include "ns3/ptr.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-routing-protocol.h"

namespace ns3 {
namespace ospf {

// class Packet;
// class NetDevice;
// class Ipv6Interface;
// class Ipv6Route;
// class Node;
// class Ipv6RoutingTableEntry;
// class Ipv6MulticastRoutingTableEntry;

/**
 * \ingroup ipv6Routing
 *
 * \brief Static routing protocol for IP version 6 stacks.
 *
 * This class provides a basic set of methods for inserting static
 * unicast and multicast routes into the Ipv6 routing system.
 * This particular protocol is designed to be inserted into an
 * Ipv6ListRouting protocol but can be used also as a standalone
 * protocol.
 *
 * The Ipv6OspfRouting class inherits from the abstract base class
 * Ipv6RoutingProtocol that defines the interface methods that a routing
 * protocol must support.
 *
 * \see Ipv6RoutingProtocol
 * \see Ipv6ListRouting
 * \see Ipv6ListRouting::AddRoutingProtocol
 */
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

  // インターフェースにアドレスが追加されると呼ばれる
  virtual void NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address);

  // インターフェースからアドレスが削除されると呼ばれる
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address);

  // ICMPv6などで経路の更新が発生すると呼ばれる
  virtual void NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse = Ipv6Address::GetZero ());

  //
  virtual void NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse = Ipv6Address::GetZero ());
  virtual void SetIpv6 (Ptr<Ipv6> ipv6);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

  virtual void HandleProtocolMessage (Ptr<Socket> socket);

protected:
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();

private:
  Time HelloInterval;
  Time DeadInterval;

  typedef std::map< Ptr<Socket>, Ipv6InterfaceAddress > ProtoSocketAddrs;

  /**
   * \brief Lookup in the forwarding table for destination.
   * \param dest destination address
   * \param interface output interface if any (put 0 otherwise)
   * \return Ipv6Route to route the packet to reach dest address
   */
  Ptr<Ipv6Route> Lookup (Ipv6Address dest, Ptr<NetDevice> = 0);

  /**
   * \brief Lookup in the multicast forwarding table for destination.
   * \param origin source address
   * \param group group multicast address
   * \param ifIndex interface index
   * \return Ipv6MulticastRoute to route the packet to reach dest address
   */
  // Ptr<Ipv6MulticastRoute> LookupStatic (Ipv6Address origin, Ipv6Address group, uint32_t ifIndex);

  /**
   * \brief the forwarding table for network.
   */
  RoutingTable m_routingTable;

  /**
   * \brief the forwarding table for multicast.
   */
  // MulticastRoutes m_multicastRoutes;

  /**
   * \brief Ipv6 reference.
   */
  Ptr<Ipv6> m_ipv6;

  ProtoSocketAddrs m_socketAddresses;
};

} /* namespace ospf */
} /* namespace ns3 */

#endif /* IPV6_STATIC_ROUTING_H */

