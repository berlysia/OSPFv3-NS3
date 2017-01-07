#include <iomanip>
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/net-device.h"
#include "ns3/names.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/inet6-socket-address.h"

#include "ipv6-ospf-routing.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE ("Ipv6OspfRouting");

NS_OBJECT_ENSURE_REGISTERED (Ipv6OspfRouting);

TypeId Ipv6OspfRouting::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::ospf::Ipv6OspfRouting")
                        .SetParent<Ipv6RoutingProtocol> ()
                        .SetGroupName ("Internet")
                        .AddConstructor<Ipv6OspfRouting> ();
    return tid;
}

Ipv6OspfRouting::Ipv6OspfRouting ()
    : m_ipv6 (0)
{
    NS_LOG_FUNCTION_NOARGS ();
    ROUTER_ID = ROUTER_ID_SEED++;
}

Ipv6OspfRouting::~Ipv6OspfRouting ()
{
    NS_LOG_FUNCTION_NOARGS ();
}

void Ipv6OspfRouting::Start() {
    for (int i = 0, l = m_ipv6->GetNInterfaces (); i < l; i++)
    {
        if (m_ipv6->IsUp (i))
        {
            NotifyInterfaceUp (i);
        }
        else
        {
            NotifyInterfaceDown (i);
        }
    }
}

void Ipv6OspfRouting::SetIpv6 (Ptr<Ipv6> ipv6)
{
    NS_LOG_FUNCTION (this << ipv6);
    NS_ASSERT (m_ipv6 == 0 && ipv6 != 0);
    uint32_t i = 0;
    m_ipv6 = ipv6;

    Simulator::ScheduleNow(&Ipv6OspfRouting::Start, this);
}

// Formatted like output of "route -n" command
void Ipv6OspfRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
    NS_LOG_FUNCTION (this << stream);
    std::ostream* os = stream->GetStream ();

    *os << "Node: " << m_ipv6->GetObject<Node> ()->GetId ()
        << ", Time: " << Now().As (Time::S)
        << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (Time::S)
        << ", Ipv6OspfRouting table" << std::endl;

    // if (GetNRoutes () > 0)
    // {
    //     *os << "Destination                    Next Hop                   Flag Met Ref Use If" << std::endl;
    //     for (uint32_t j = 0; j < GetNRoutes (); j++)
    //     {
    //         std::ostringstream dest, gw, mask, flags;
    //         const Ipv6RoutingTableEntry &route = GetRoute (j);
    //         dest << route.GetDest () << "/" << int(route.GetDestNetworkPrefix ().GetPrefixLength ());
    //         *os << std::setiosflags (std::ios::left) << std::setw (31) << dest.str ();
    //         gw << route.GetGateway ();
    //         *os << std::setiosflags (std::ios::left) << std::setw (27) << gw.str ();
    //         flags << "U";
    //         if (route.IsHost ())
    //         {
    //             flags << "H";
    //         }
    //         else if (route.IsGateway ())
    //         {
    //             flags << "G";
    //         }
    //         *os << std::setiosflags (std::ios::left) << std::setw (5) << flags.str ();
    //         *os << std::setiosflags (std::ios::left) << std::setw (4) << GetMetric (j);
    //         // Ref ct not implemented
    //         *os << "-" << "   ";
    //         // Use not implemented
    //         *os << "-" << "   ";
    //         if (Names::FindName (m_ipv6->GetNetDevice (route.GetInterface ())) != "")
    //         {
    //             *os << Names::FindName (m_ipv6->GetNetDevice (route.GetInterface ()));
    //         }
    //         else
    //         {
    //             *os << route.GetInterface ();
    //         }
    //         *os << std::endl;
    //     }
    // }
    *os << std::endl;
}

Ptr<Ipv6Route> Ipv6OspfRouting::RouteOutput (
    Ptr<Packet> p,
    const Ipv6Header &header,
    Ptr<NetDevice> oif,
    Socket::SocketErrno &sockerr) {

    return Ptr<Ipv6Route>();
}

bool Ipv6OspfRouting::RouteInput (
    Ptr<const Packet> p,
    const Ipv6Header &header,
    Ptr<const NetDevice> idev,
    UnicastForwardCallback ucb,
    MulticastForwardCallback mcb,
    LocalDeliverCallback lcb,
    ErrorCallback ecb) {
    return false;
}

void Ipv6OspfRouting::NotifyInterfaceUp (uint32_t interface) {

}

void Ipv6OspfRouting::NotifyInterfaceDown (uint32_t interface) {

}

void Ipv6OspfRouting::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address) {

}

void Ipv6OspfRouting::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address) {

}

void Ipv6OspfRouting::NotifyAddRoute (
    Ipv6Address dst,
    Ipv6Prefix mask,
    Ipv6Address nextHop,
    uint32_t interface,
    Ipv6Address prefixToUse) {

}

void Ipv6OspfRouting::NotifyRemoveRoute (
    Ipv6Address dst,
    Ipv6Prefix mask,
    Ipv6Address nextHop,
    uint32_t interface,
    Ipv6Address prefixToUse) {

}


void Ipv6OspfRouting::DoDispose ()
{
    NS_LOG_FUNCTION_NOARGS ();

    // for (NetworkRoutesI j = m_networkRoutes.begin ();  j != m_networkRoutes.end (); j = m_networkRoutes.erase (j))
    // {
    //     delete j->first;
    // }
    // m_networkRoutes.clear ();

    // for (MulticastRoutesI i = m_multicastRoutes.begin (); i != m_multicastRoutes.end (); i = m_multicastRoutes.erase (i))
    // {
    //     delete (*i);
    // }
    // m_multicastRoutes.clear ();

    m_ipv6 = 0;
    Ipv6RoutingProtocol::DoDispose ();
}

void Ipv6OspfRouting::HandleProtocolMessage(Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this);

    uint32_t ifaceIdx = m_socketToIfaceIdx[socket];
    InterfaceData &data = m_interfaces[ifaceIdx];

    // 1. checksumが正しいか(ignore)
    // 2. dstAddrが受け取ったIfaceのアドレス、またはマルチキャストのAllSPFRouters, AllDRouters、あるいはグローバルアドレスか
    // 3. IP Protocolが89になっているか
    // 4. 自分が送信したパケットでないか

    // - AreaIdが正しいか(ignore)
    // - AllDRouters宛のとき、受け取ったIfaceがDRまたはBDRか
    // - AuTypeはそのエリアで有効なものか(ignore)
    // - パケットが認証されているか(ignore)
}

/*

Ptr<Ipv6Route> Ipv6OspfRouting::RouteOutput (
    Ptr<Packet> p,
    const Ipv6Header &header,
    Ptr<NetDevice> oif,
    Socket::SocketErrno &sockerr)
{
    NS_LOG_FUNCTION (this << header << oif);
    Ipv6Address destination = header.GetDestinationAddress ();
    Ptr<Ipv6Route> rtentry = 0;

    if (destination.IsMulticast ())
    {
        // Note:  Multicast routes for outbound packets are stored in the
        // normal unicast table.  An implication of this is that it is not
        // possible to source multicast datagrams on multiple interfaces.
        // This is a well-known property of sockets implementation on
        // many Unix variants.
        // So, we just log it and fall through to Lookup ()
        NS_LOG_LOGIC ("RouteOutput ()::Multicast destination");
    }

    rtentry = Lookup (destination, oif);
    if (rtentry)
    {
        sockerr = Socket::ERROR_NOTERROR;
    }
    else
    {
        sockerr = Socket::ERROR_NOROUTETOHOST;
    }
    return rtentry;
}

bool Ipv6OspfRouting::RouteInput (
    Ptr<const Packet> p,
    const Ipv6Header &header,
    Ptr<const NetDevice> idev,
    UnicastForwardCallback ucb,
    MulticastForwardCallback mcb,
    LocalDeliverCallback lcb,
    ErrorCallback ecb)
{
    NS_LOG_FUNCTION (this << p << header << header.GetSourceAddress () << header.GetDestinationAddress () << idev);
    NS_ASSERT (m_ipv6 != 0);
    // Check if input device supports IP
    NS_ASSERT (m_ipv6->GetInterfaceForDevice (idev) >= 0);
    uint32_t iif = m_ipv6->GetInterfaceForDevice (idev);
    Ipv6Address dst = header.GetDestinationAddress ();

    // Multicast recognition; handle local delivery here
    if (dst.IsMulticast ())
    {
        NS_LOG_LOGIC ("Multicast destination");
        // Ptr<Ipv6MulticastRoute> mrtentry = Lookup (header.GetSourceAddress (),
        //                                    header.GetDestinationAddress (), m_ipv6->GetInterfaceForDevice (idev));

        // // \todo check if we want to forward up the packet
        // if (mrtentry)
        // {
        //     NS_LOG_LOGIC ("Multicast route found");
        //     mcb (idev, mrtentry, p, header); // multicast forwarding callback
        //     return true;
        // }
        // else
        // {
        //     NS_LOG_LOGIC ("Multicast route not found");
        //     return false; // Let other routing protocols try to handle this
        // }
        return false;
    }

    // Check if input device supports IP forwarding
    if (m_ipv6->IsForwarding (iif) == false)
    {
        NS_LOG_LOGIC ("Forwarding disabled for this interface");
        if (!ecb.IsNull ())
        {
            ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
        return true;
    }
    // Next, try to find a route
    NS_LOG_LOGIC ("Unicast destination");
    Ptr<Ipv6Route> rtentry = Lookup (header.GetDestinationAddress ());

    if (rtentry != 0)
    {
        NS_LOG_LOGIC ("Found unicast destination- calling unicast callback");
        ucb (idev, rtentry, p, header);  // unicast forwarding callback
        return true;
    }
    else
    {
        NS_LOG_LOGIC ("Did not find unicast destination- returning false");
        return false; // Let other routing protocols try to handle this
    }
}

void Ipv6OspfRouting::NotifyInterfaceUp (uint32_t i)
{
    NS_LOG_FUNCTION(this << m_ipv6->GetAddress(i, 0).GetAddress());

    // add routes

    for (uint32_t j = 0, l = m_ipv6->GetNAddresses (i); j < l; ++j) {
        Ipv6Address addr = &m_ipv6->GetAddress (i, j);

        if (addr.GetAddress () != Ipv6Address ()
            && addr.GetPrefix () != Ipv6Prefix ())
        {
            switch(addr.GetPrefix ().GetScope()) {
                case Ipv6InterfaceAddress::HOST: {
                    AddHostRouteTo (addr.GetAddress (), i);
                } break;
                case Ipv6InterfaceAddress::LINKLOCAL: {
                    AddNetworkRouteTo (addr.GetAddress ().CombinePrefix (addr.GetPrefix ()),
                    addr.GetPrefix (), i);
                }
                default: {
                    NS_LOG_LOGIC("found GLOBAL address: " << addr);
                }
            }
        }
    }

    Ptr<Ipv6L3Protocol> l3 = m_ipv6->GetObject<Ipv6L3Protocol>();
    Ipv6InterfaceAddress ifaceAddr = l3->GetAddress(i, 0);

    // プロトコル通信用socket生成
    Ptr<Socket> socket = Socket::CreateSocket(
        GetObject<Node>(),
        Ipv6RawSocketFactory::GetTypeId()
    );
    NS_ASSERT(socket != 0);

    socket->SetRecvCallback(MakeCallback(&Ipv6OspfRouting::HandleProtocolMessage, this));
    socket->BindToNetDevice(l3->GetNetDevice(i));
    Inet6SocketAddress localAddr = Inet6SocketAddress(ifaceAddr.GetAddress(), PROTO_PORT);
    socket->Bind(localAddr);
    socket->SetProtocol(89); // OSPFv3
    m_socketAddresses.insert(std::make_pair(socket, ifaceAddr));
}

void Ipv6OspfRouting::NotifyInterfaceDown (uint32_t i)
{
    NS_LOG_FUNCTION (this << i);

    // remove all static routes that are going through this interface
    for (NetworkRoutesI it = m_networkRoutes.begin (); it != m_networkRoutes.end (); )
    {
        if (it->first->GetInterface () == i)
        {
            delete it->first;
            it = m_networkRoutes.erase (it);
        }
        else
        {
            it++;
        }
    }
}

void Ipv6OspfRouting::HandleProtocolMessage(Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this);

    uint32_t ifaceIdx = m_socketToIfaceIdx[socket];
    InterfaceData &data = m_interfaces[ifaceIdx];
}

void Ipv6OspfRouting::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
    if (!m_ipv6->IsUp (interface))
    {
        return;
    }

    Ipv6Address networkAddress = address.GetAddress ().CombinePrefix (address.GetPrefix ());
    Ipv6Prefix networkMask = address.GetPrefix ();

    if (address.GetAddress () != Ipv6Address () && address.GetPrefix () != Ipv6Prefix ())
    {
        AddNetworkRouteTo (networkAddress, networkMask, interface);
    }
}

void Ipv6OspfRouting::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
    if (!m_ipv6->IsUp (interface))
    {
        return;
    }

    Ipv6Prefix networkMask = address.GetPrefix ();
    Ipv6Address networkAddress = address.GetAddress ().CombinePrefix (networkMask);

    // Remove all static routes that are going through this interface
    // which reference this network
    for (NetworkRoutesI it = m_networkRoutes.begin (); it != m_networkRoutes.end (); )
    {
        if (it->first->GetInterface () == interface
                && it->first->IsNetwork ()
                && it->first->GetDestNetwork () == networkAddress
                && it->first->GetDestNetworkPrefix () == networkMask)
        {
            delete it->first;
            it = m_networkRoutes.erase (it);
        }
        else
        {
            it++;
        }
    }
}

void Ipv6OspfRouting::NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
{
    NS_LOG_INFO (this << dst << mask << nextHop << interface << prefixToUse);
    if (dst != Ipv6Address::GetZero ())
    {
        AddNetworkRouteTo (dst, mask, nextHop, interface);
    }
    else //default route
    {
        // this case is mainly used by configuring default route following RA processing,
        // in case of multiple prefix in RA, the first will configured default route

        // for the moment, all default route has the same metric
        // so according to the longest prefix algorithm,
        // the default route chosen will be the last added
 
        SetDefaultRoute (nextHop, interface, prefixToUse);
    }
}

void Ipv6OspfRouting::NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
{
    NS_LOG_FUNCTION (this << dst << mask << nextHop << interface);
    if (dst != Ipv6Address::GetZero ())
    {
        for (NetworkRoutesI j = m_networkRoutes.begin (); j != m_networkRoutes.end ();)
        {
            Ipv6RoutingTableEntry* rtentry = j->first;
            Ipv6Prefix prefix = rtentry->GetDestNetworkPrefix ();
            Ipv6Address entry = rtentry->GetDestNetwork ();

            if (dst == entry && prefix == mask && rtentry->GetInterface () == interface)
            {
                delete j->first;
                j = m_networkRoutes.erase (j);
            }
            else
            {
                ++j;
            }
        }
    }
    else
    {
        RemoveRoute (dst, mask, interface, prefixToUse);
    }
}
*/

} /* namespace ospf */
} /* namespace ns3 */