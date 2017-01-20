#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <climits>
#include <algorithm>
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/net-device.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/names.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/uinteger.h"
#include "ns3/data-rate.h"

#include "ipv6-ospf-routing.h"

#include "ospf-hello.h"
#include "ospf-database-description.h"
#include "ospf-link-state-request.h"
#include "ospf-link-state-update.h"
#include "ospf-link-state-ack.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE ("Ipv6OspfRouting");

NS_OBJECT_ENSURE_REGISTERED (Ipv6OspfRouting);

const Ipv6Address Ipv6OspfRouting::AllSPFRouters = Ipv6Address("ff02::5");
const Ipv6Address Ipv6OspfRouting::AllDRRouters = Ipv6Address("ff02::6");
uint32_t Ipv6OspfRouting::ROUTER_ID_SEED = 1;

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
    NS_LOG_FUNCTION (m_routerId);
    // m_routingTable.SetRouterId(m_routerId);
}

Ipv6OspfRouting::~Ipv6OspfRouting ()
{
    NS_LOG_FUNCTION (m_routerId);
}

void Ipv6OspfRouting::Start() {
    m_interfaces.resize(m_ipv6->GetNInterfaces ());
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
    NS_LOG_FUNCTION (ipv6);
    NS_ASSERT (m_ipv6 == 0 && ipv6 != 0);
    m_ipv6 = ipv6;
    m_routerId = m_ipv6->GetObject<Node> ()->GetId () + 1; // non-zero
    m_knownMaxRouterId = m_routerId;

    NS_LOG_INFO("router " << m_routerId << " is configured");

    Simulator::ScheduleNow(&Ipv6OspfRouting::Start, this);
}

// Formatted like output of "route -n" command
void Ipv6OspfRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
    NS_LOG_FUNCTION (m_routerId << stream);
    std::ostream* os = stream->GetStream ();

    *os << "Router: " << m_routerId
        << ", Time: " << Now().As (Time::S)
        << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (Time::S)
        << ", Ipv6OspfRouting table" << std::endl;
    
    *os << m_routingTable << std::flush;

    // if (GetNRoutes () > 0)
    // {
    //     *os << "Destination                    Next Hop                   Flag Met Ref Use If" << std::endl;
    //     for (uint32_t j = 0; j < GetNRoutes (); j++)
    //     {
    //         std::ostringstream dest, gw, mask, flags;
    //         const Ipv6RoutingTableEntry &route = GetRoute (j);();
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
    Socket::SocketErrno &sockerr
) {

    NS_LOG_FUNCTION(m_routerId << p << header << oif);
    NS_LOG_INFO("packet: " << p->ToString());
    if (p->GetSize() == 0) return 0;

    Ipv6Address destination = header.GetDestinationAddress ();
    Ipv6Address source = header.GetSourceAddress ();
    NS_LOG_INFO("src->dst: " << source << " -> " << destination);
    Ptr<Ipv6Route> rtentry = 0;

    if (destination.IsLinkLocalMulticast() && oif) {
        rtentry = Create<Ipv6Route>();
        rtentry->SetSource(m_ipv6->SourceAddressSelection (m_ipv6->GetInterfaceForDevice (oif), destination));
        // rtentry->SetSource(m_interfaces[oif->GetIfIndex()].GetAddress());
        rtentry->SetDestination(destination);
        rtentry->SetGateway(Ipv6Address::GetZero());
        rtentry->SetOutputDevice(oif);
        NS_LOG_LOGIC ("RouteOutput ()::LinkLocalMulticast destination - " << *rtentry);
        return rtentry;
    }

    if (destination.IsLinkLocal()) {
        rtentry = Create<Ipv6Route>();
        rtentry->SetDestination(destination);
        rtentry->SetGateway(Ipv6Address::GetZero());
        if (oif) {
            rtentry->SetSource(m_ipv6->SourceAddressSelection (m_ipv6->GetInterfaceForDevice (oif), destination));
            rtentry->SetOutputDevice(oif);
            NS_LOG_LOGIC ("RouteOutput ()::LinkLocal destination - " << *rtentry);
            return rtentry;
        } else {
            for (InterfaceData& ifaceData : m_interfaces) {
                for (auto& kv : ifaceData.GetNeighbors()) {
                    // NS_LOG_LOGIC("router: " << m_routerId << ", iface: " << ifaceData.GetInterfaceId() << ", ifaceAddr: " << ifaceData.GetAddress() << ", neighAddr: " << kv.second.GetAddress());
                    // if (ifaceData.GetAddress() == destination) {
                    //     rtentry->SetSource(ifaceData.GetAddress());
                    //     rtentry->SetOutputDevice(0); // loopback

                    //     NS_LOG_LOGIC ("RouteOutput ()::LinkLocal destination - loopback! " << *rtentry);
                    //     return rtentry;
                    // }
                    if (kv.second.GetAddress() == destination) {
                        rtentry->SetSource(ifaceData.GetAddress());
                        rtentry->SetOutputDevice(m_ipv6->GetNetDevice(ifaceData.GetInterfaceId()));

                        NS_LOG_LOGIC ("RouteOutput ()::LinkLocal destination - " << *rtentry);
                        return rtentry;
                    }
                }
            }
            NS_LOG_LOGIC ("RouteOutput ()::LinkLocal destination - not found");
        }
    }

    if (destination.IsMulticast ())
    {

        NS_LOG_LOGIC ("RouteOutput ()::Multicast destination");
        // nop
    }

    rtentry = Lookup (source, destination);
    if (rtentry)
    {
        sockerr = Socket::ERROR_NOTERROR;
        NS_LOG_LOGIC ("RouteOutput ():: route found" << rtentry);
    }
    else
    {
        sockerr = Socket::ERROR_NOROUTETOHOST;
        NS_LOG_LOGIC ("RouteOutput ():: no route to host");
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
    ErrorCallback ecb
) {
    NS_LOG_FUNCTION (m_routerId << this << p << header << header.GetSourceAddress () << header.GetDestinationAddress () << idev);
    NS_ASSERT (m_ipv6 != 0);
    // Check if input device supports IP
    NS_ASSERT (m_ipv6->GetInterfaceForDevice (idev) >= 0);
    uint32_t iif = m_ipv6->GetInterfaceForDevice (idev);
    Ipv6Address dst = header.GetDestinationAddress ();

    if (dst.IsLinkLocalMulticast()) {
        // NS_LOG_LOGIC("Link Local Multicast packet received");
        // if (!lcb.IsNull()) {
        //     NS_LOG_LOGIC("localDeriverCallback - call");
        //     lcb(p, header, Socket::ERROR_NOTERROR);
        //     NS_LOG_LOGIC("localDeriverCallback - end");
        // }
        return true;
    }

    if (dst.IsLinkLocal()) {
        NS_LOG_LOGIC("Link Local but not Multicast packet received. fall through");
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
    Ptr<Ipv6Route> rtentry = Lookup (header.GetSourceAddress (), header.GetDestinationAddress ());
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

    return false;
}

void Ipv6OspfRouting::NotifyInterfaceUp (uint32_t ifaceIdx) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << m_ipv6->GetAddress(ifaceIdx, 0).GetAddress());
    if (ifaceIdx == 0) {
        return;
    }

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];

    ifaceData.SetInterfaceId(ifaceIdx);
    m_rtrIfaceId_set.insert(ifaceIdx);

    ifaceData.SetType(InterfaceType::P2P); // FIXME: 決め打ちしている

    Timer& waitTimer = ifaceData.GetWaitTimer();
    waitTimer.SetFunction(&Ipv6OspfRouting::NotifyInterfaceEvent, this);
    waitTimer.SetArguments(ifaceIdx, InterfaceEvent::WAIT_TIMER);

    Timer& helloTimer = ifaceData.GetHelloTimer();
    helloTimer.SetFunction(&Ipv6OspfRouting::SendHelloPacket, this);
    helloTimer.SetArguments(ifaceIdx);
    Simulator::ScheduleNow(&Ipv6OspfRouting::SendHelloPacket, this, ifaceIdx);

    Ptr<Ipv6L3Protocol> l3 = m_ipv6->GetObject<Ipv6L3Protocol>();
    Ipv6InterfaceAddress ifaceAddr = l3->GetAddress(ifaceIdx, 0); // Link Local

    ifaceData.SetAddress(ifaceAddr.GetAddress());
    ifaceData.SetPrefix(ifaceAddr.GetPrefix());

    // プロトコル送信用socket生成
    Ptr<Socket> socket = Socket::CreateSocket(
        m_ipv6->GetObject<Node>(),
        Ipv6RawSocketFactory::GetTypeId()
    );
    NS_ASSERT(socket);

    socket->SetRecvCallback(MakeCallback(&Ipv6OspfRouting::HandleProtocolMessage, this));
    Inet6SocketAddress localAddr = Inet6SocketAddress(ifaceAddr.GetAddress(), PROTO_PORT);
    NS_LOG_INFO(ifaceAddr);
    socket->Bind(localAddr);
    socket->SetAttribute("Protocol", UintegerValue(89)); // OSPFv3
    socket->SetAllowBroadcast(true);
    m_socketToIfaceIdx[socket] = ifaceIdx;
    m_ifaceIdxToSocket[ifaceIdx] = socket;

    // プロトコル受信用socket生成
    Ptr<Socket> rcv_socket = Socket::CreateSocket(
        m_ipv6->GetObject<Node>(),
        Ipv6RawSocketFactory::GetTypeId()
    );
    NS_ASSERT(rcv_socket);

    rcv_socket->SetRecvCallback(MakeCallback(&Ipv6OspfRouting::HandleProtocolMessage, this));
    Inet6SocketAddress llmAddr = Inet6SocketAddress(Ipv6OspfRouting::AllSPFRouters, PROTO_PORT);
    NS_LOG_INFO(ifaceAddr);
    rcv_socket->Bind(llmAddr);
    rcv_socket->BindToNetDevice(l3->GetNetDevice(ifaceIdx));
    rcv_socket->SetAllowBroadcast(true);
    rcv_socket->SetAttribute("Protocol", UintegerValue(89)); // OSPFv3
    m_llmSocketToIfaceIdx[rcv_socket] = ifaceIdx;
    m_ifaceIdxToLlmSocket[ifaceIdx] = rcv_socket;

    NotifyInterfaceEvent(ifaceIdx, InterfaceEvent::IF_UP);
}

void Ipv6OspfRouting::NotifyInterfaceDown (uint32_t ifaceIdx) {
    NS_LOG_FUNCTION (m_routerId << ifaceIdx);

    NotifyInterfaceEvent(ifaceIdx, InterfaceEvent::IF_DOWN);
}

void Ipv6OspfRouting::NotifyAddAddress (uint32_t ifaceIdx, Ipv6InterfaceAddress address) {
    NS_LOG_FUNCTION (m_routerId << ifaceIdx << address);
    NS_LOG_ERROR (this << " - unimplemented");
}

void Ipv6OspfRouting::NotifyRemoveAddress (uint32_t ifaceIdx, Ipv6InterfaceAddress address) {
    NS_LOG_FUNCTION (m_routerId << ifaceIdx << address);
    NS_LOG_ERROR (this << " - unimplemented");
}

void Ipv6OspfRouting::NotifyAddRoute (
    Ipv6Address dst,
    Ipv6Prefix mask,
    Ipv6Address nextHop,
    uint32_t ifaceIdx,
    Ipv6Address prefixToUse
) {
    NS_LOG_FUNCTION (m_routerId << dst << mask << nextHop);
    NS_LOG_ERROR (this << " - unimplemented");
}

void Ipv6OspfRouting::NotifyRemoveRoute (
    Ipv6Address dst,
    Ipv6Prefix mask,
    Ipv6Address nextHop,
    uint32_t ifaceIdx,
    Ipv6Address prefixToUse
) {
    NS_LOG_FUNCTION (m_routerId << dst << mask << nextHop);
    NS_LOG_ERROR (this << " - unimplemented");
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
    // NS_LOG_FUNCTION (this << socket);

    // 1. checksumが正しいか(ignore)
    // 2. dstAddrが受け取ったIfaceのアドレス、またはマルチキャストのAllSPFRouters, AllDRouters、あるいはグローバルアドレスか
    // 3. IP Protocolが89になっているか
    // 4. 自分が送信したパケットでないか

    // - AreaIdが正しいか(ignore)
    // - AllDRouters宛のとき、受け取ったIfaceがDRまたはBDRか
    // - AuTypeはそのエリアで有効なものか(ignore)
    // - パケットが認証されているか(ignore)

    // 各パケットごとの受け取り実装に移譲する

    uint32_t ifaceIdx = m_socketToIfaceIdx.count(socket) ? m_socketToIfaceIdx[socket] : m_llmSocketToIfaceIdx[socket];
    NS_LOG_FUNCTION (m_routerId);
    NS_LOG_INFO("routerID: " << m_routerId << ", ifaceIdx: " << ifaceIdx);

    Address pctSrcAddr;
    Ptr<Packet> packet = socket->RecvFrom(pctSrcAddr);

    Ipv6Header ipv6Header;
    packet->RemoveHeader(ipv6Header);

    Inet6SocketAddress inet6SrcAddr = Inet6SocketAddress::ConvertFrom(pctSrcAddr);
    Ipv6Address srcAddr = inet6SrcAddr.GetIpv6();

    OSPFHeader ospfHeader;
    packet->PeekHeader(ospfHeader);

    NS_LOG_LOGIC("header: received ifaceIdx " << ifaceIdx << ", srcAddr: " << srcAddr);

    uint8_t ospfPacketType = ospfHeader.GetType();
    switch (ospfPacketType) {
        case OSPF_TYPE_HELLO: 
            NS_LOG_LOGIC("received OSPF Header type: Hello");
            return ReceiveHelloPacket(ifaceIdx, srcAddr, packet);
        case OSPF_TYPE_DATABASE_DESCRIPTION: 
            NS_LOG_LOGIC("received OSPF Header type: DatabaseDescription");
            return ReceiveDatabaseDescriptionPacket(ifaceIdx, packet);
        case OSPF_TYPE_LINK_STATE_REQUEST:
            NS_LOG_LOGIC("received OSPF Header type: LinkStateRequest");
            return ReceiveLinkStateRequestPacket(ifaceIdx, packet);
        case OSPF_TYPE_LINK_STATE_UPDATE:
            NS_LOG_LOGIC("received OSPF Header type: LinkStateUpdate");
            return ReceiveLinkStateUpdatePacket(ifaceIdx, packet);
        case OSPF_TYPE_LINK_STATE_ACK:
            NS_LOG_LOGIC("received OSPF Header type: LinkStateAck");
            return ReceiveLinkStateAckPacket(ifaceIdx, packet);
        default:
            NS_LOG_WARN("unknown ospf packet type: " << ospfPacketType);
    }

}

void Ipv6OspfRouting::RegisterToLSDB(Ptr<OSPFLSA> lsa) {
    NS_LOG_FUNCTION(m_routerId << *lsa);
    UpdateLSACaches(lsa);
    m_lsdb.Add(lsa);
}

void Ipv6OspfRouting::UpdateLSACaches(Ptr<OSPFLSA> lsa) {
    RouterId advRtr = lsa->GetHeader()->GetAdvertisingRouter();
    if (m_knownMaxRouterId < advRtr) {
        NS_LOG_INFO("m_knownMaxRouterId for " << m_routerId << " is updated: " << m_knownMaxRouterId << " -> " << advRtr);
        m_knownMaxRouterId = advRtr;
    }

    switch (lsa->GetHeader()->GetType()) {
        case OSPF_LSA_TYPE_LINK: {
            int32_t idx = GetInterfaceForNeighbor(advRtr);
            if (idx >= 0 && idx < m_interfaces.size()) {
                m_interfaces[idx].AddLinkLocalLSA(advRtr, lsa);
            } else {
                NS_LOG_ERROR("### failed to register Link-LSA ### router: " << m_routerId << ", advRtr: " << advRtr);
            }
        } break;
        case OSPF_LSA_TYPE_ROUTER: {
            m_routerLSA_set.insert(lsa->GetIdentifier());
            auto& body = *lsa->GetBody<OSPFRouterLSABody>();
            for (int i = 0, l = body.CountNeighbors(); i < l; ++i) {
                RouterId neighborId = body.GetNeighborRouterId(i);
                if (m_knownMaxRouterId < neighborId) {
                    NS_LOG_INFO("m_knownMaxRouterId for " << m_routerId << " is updated: " << m_knownMaxRouterId << " -> " << neighborId);
                    m_knownMaxRouterId = neighborId;
                }                
            }

        } break;
        case OSPF_LSA_TYPE_INTRA_AREA_PREFIX: {
            m_intraAreaPrefixLSA_set.insert(lsa->GetIdentifier());
        } break;
    }
}

void Ipv6OspfRouting::OriginateLinkLSA(uint32_t ifaceIdx, bool forceRefresh) {
    NS_LOG_FUNCTION (m_routerId << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFLinkStateIdentifier id(OSPF_LSA_TYPE_LINK, ifaceData.GetInterfaceId(), m_routerId);
    // if (m_lsdb.Has(id) && !forceRefresh) {
    //     NS_LOG_LOGIC("インスタンス再生成のみ");
    //     OSPFLSA& lsa = m_lsdb.Get(id);
    //     lsa.GetHeader()->SetAge(0);
    //     lsa.GetHeader()->IncrementSequenceNumber();

    //     AppendToRxmtList(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
    //     return;
    // }

    Ptr<OSPFLSA> lsa;
    if (m_lsdb.Has(id)) {
        NS_LOG_LOGIC("インスタンス更新 - Link-LSA for " << m_routerId);
        lsa = m_lsdb.Get(id);
        OSPFLSAHeader& hdr = *lsa->GetHeader();
        hdr.SetAge(0);
        hdr.IncrementSequenceNumber();
    } else {
        NS_LOG_LOGIC("新規インスタンス作成 - Link-LSA for " << m_routerId);
        lsa = Ptr<OSPFLSA>(new OSPFLSA());
        lsa->Initialize(OSPF_LSA_TYPE_LINK);
        OSPFLSAHeader& hdr = *lsa->GetHeader();
        hdr.SetAge(0);
        hdr.InitializeSequenceNumber();
        hdr.SetId(ifaceData.GetInterfaceId());
        hdr.SetAdvertisingRouter(m_routerId);
        // hdr.SetChecksum(uint16_t);
        // hdr.SetLength(uint16_t);
    }

    OSPFLinkLSABody& body = *lsa->GetBody<OSPFLinkLSABody>();
    body.SetRtrPriority(1);
    body.SetOptions(0x13); // V6, E, R
    body.SetLinkLocalAddress(ifaceData.GetAddress());
    body.ClearPrefixes();

    uint8_t options;
    for (uint32_t i = 0, l = m_ipv6->GetNAddresses (ifaceIdx); i < l; ++i) {
        Ipv6InterfaceAddress addr = m_ipv6->GetAddress (ifaceIdx, i);
        if (
            addr.GetAddress () != Ipv6Address () &&
            addr.GetPrefix () != Ipv6Prefix () &&
            addr.GetScope() == Ipv6InterfaceAddress::GLOBAL
        ) {
            options = 0x0; // リンクに存在する唯一のルータの場合は0x2とし、プレフィクス長も128bitにする（ホストアドレス扱い）
            body.AddPrefix(addr.GetAddress(), addr.GetPrefix().GetPrefixLength(), options);
            NS_LOG_INFO("add prefix: " << addr.GetAddress() << addr.GetPrefix());
        }
    }

    NS_LOG_INFO("Link-LSA for #" << m_routerId << " result: " << *lsa);

    bool updateFlag = true;
    if (m_lsdb.Has(id)) {
        updateFlag = !(*m_lsdb.Get(id)->GetBody() == *lsa->GetBody());
    } else {
        RegisterToLSDB(lsa);
    }
    UpdateLSACaches(lsa);
    RemoveFromAllRxmtList(id);
    AppendToRxmtList(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
    if (updateFlag) {
        if (m_tableUpdateReducible) {
            m_tableUpdateRequired = true;
        } else {
            CalcRoutingTable();
        }
    }
}
void Ipv6OspfRouting::OriginateRouterLSA(bool forceRefresh) {
    NS_LOG_FUNCTION (m_routerId);
    OSPFLinkStateIdentifier id(OSPF_LSA_TYPE_ROUTER, m_routerId, m_routerId);
    // if (m_lsdb.Has(id) && !forceRefresh) {
    //     NS_LOG_LOGIC("インスタンス再生成のみ");
    //     OSPFLSA& lsa = m_lsdb.Get(id);
    //     lsa.GetHeader()->SetAge(0);
    //     lsa.GetHeader()->IncrementSequenceNumber();

    //     AppendToRxmtList(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
    //     return;
    // }

    Ptr<OSPFLSA> lsa;
    if (m_lsdb.Has(id)) {
        NS_LOG_LOGIC("インスタンス更新 - Router-LSA for " << m_routerId);
        lsa = m_lsdb.Get(id);
        OSPFLSAHeader& hdr = *lsa->GetHeader();
        hdr.SetAge(0);
        hdr.IncrementSequenceNumber();
    } else {
        NS_LOG_LOGIC("新規インスタンス作成 - Router-LSA for " << m_routerId);
        lsa = Ptr<OSPFLSA>(new OSPFLSA());
        lsa->Initialize(OSPF_LSA_TYPE_ROUTER);
        OSPFLSAHeader& hdr = *lsa->GetHeader();
        hdr.SetAge(0);
        hdr.InitializeSequenceNumber();
        hdr.SetId(m_routerId);
        hdr.SetAdvertisingRouter(m_routerId);
        // hdr.SetChecksum(uint16_t);
        // hdr.SetLength(uint16_t);
    }

    OSPFRouterLSABody& body = *lsa->GetBody<OSPFRouterLSABody>();

    body.SetOptions(0);
    body.ClearNeighbors();

    uint8_t type;
    uint32_t neighIfaceId, neighRouterId;
    for (InterfaceData& ifaceData : m_interfaces) {
        if (!ifaceData.IsActive()) continue;

        uint16_t metric = CalcMetricForInterface(ifaceData.GetInterfaceId());
        switch (ifaceData.GetType()) {
            case InterfaceType::P2P: type = 1; break;
            case InterfaceType::VIRTUAL: type = 4; break;
            default: type = 2;
        }
        if (type == 2) {
            if (ifaceData.GetDesignatedRouter() == m_routerId) {
                neighIfaceId = ifaceData.GetInterfaceId();
                neighRouterId = m_routerId;
            } else {
                NeighborData& neighData = ifaceData.GetNeighbor(ifaceData.GetDesignatedRouter());
                neighIfaceId = neighData.GetInterfaceId();
                neighRouterId = neighData.GetRouterId();
            }
        } else {
            for (auto& kv : ifaceData.GetNeighbors()) {
                // type == 2でない場合、ネイバーは多くてもひとつしかないはず
                NeighborData& neighData = kv.second;
                neighIfaceId = neighData.GetInterfaceId();
                neighRouterId = neighData.GetRouterId();
                body.AddNeighbor(type, metric, ifaceData.GetInterfaceId(), neighIfaceId, neighRouterId);
                break;
            }
        }
    }

    NS_LOG_INFO("Router-LSA for #" << m_routerId << " result: " << *lsa);

    bool updateFlag = true;
    if (m_lsdb.Has(id)) {
        updateFlag = !(*m_lsdb.Get(id)->GetBody() == *lsa->GetBody());
    } else {
        RegisterToLSDB(lsa);
    }
    UpdateLSACaches(lsa);
    RemoveFromAllRxmtList(id);
    AppendToRxmtList(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
    if (updateFlag) {
        if (m_tableUpdateReducible) {
            m_tableUpdateRequired = true;
        } else {
            CalcRoutingTable();
        }
    }
}

void Ipv6OspfRouting::OriginateIntraAreaPrefixLSA(bool forceRefresh) {
    NS_LOG_FUNCTION (m_routerId);
    // 各エリアごとに実行すべきだが今の所単一エリア
    // FIXME: DR用の挙動は別の関数を書いてください

    OSPFLinkStateIdentifier id(OSPF_LSA_TYPE_INTRA_AREA_PREFIX, m_routerId, m_routerId);
    // if (m_lsdb.Has(id) && !forceRefresh) {
    //     NS_LOG_LOGIC("インスタンス再生成のみ");
    //     OSPFLSA& lsa = m_lsdb.Get(id);
    //     lsa.GetHeader()->SetAge(0);
    //     lsa.GetHeader()->IncrementSequenceNumber();

    //     AppendToRxmtList(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
    //     return;
    // }

    Ptr<OSPFLSA> lsa;
    if (m_lsdb.Has(id)) {
        NS_LOG_LOGIC("インスタンス更新 - Intra-Area-Prefix-LSA for " << m_routerId);
        lsa = m_lsdb.Get(id);
        OSPFLSAHeader& hdr = *lsa->GetHeader();
        hdr.SetAge(0);
        hdr.IncrementSequenceNumber();
    } else {
        NS_LOG_LOGIC("新規インスタンス作成 Intra-Area-Prefix-LSA for " << m_routerId);
        lsa = Ptr<OSPFLSA>(new OSPFLSA());
        lsa->Initialize(OSPF_LSA_TYPE_INTRA_AREA_PREFIX);
        OSPFLSAHeader& hdr = *lsa->GetHeader();
        hdr.SetAge(0);
        hdr.InitializeSequenceNumber();
        hdr.SetId(m_routerId); // FIXME: 
        hdr.SetAdvertisingRouter(m_routerId);
        // hdr.SetChecksum(uint16_t);
        // hdr.SetLength(uint16_t);
    }

    OSPFIntraAreaPrefixLSABody& body = dynamic_cast<OSPFIntraAreaPrefixLSABody&>(*lsa->GetBody());

    body.SetReferenceType(OSPF_LSA_TYPE_ROUTER);
    body.SetReferenceLinkStateId(0); // 0 indicates the LSA is associated with this router
    body.SetReferenceAdvertisedRouter (m_routerId);

    body.ClearPrefixes();

    // FIXME: 本来はエリアごと
    for (uint32_t ifaceIdx = 1, l = m_ipv6->GetNInterfaces (); ifaceIdx < l; ifaceIdx++) {
        InterfaceData& ifaceData = m_interfaces[ifaceIdx];
        NS_LOG_INFO("ifaceIdx: " << ifaceIdx << ", addr: " << ifaceData.GetAddress() << ", ifaceState: " << ToString(ifaceData.GetState()));
        for (uint32_t i = 0, l = m_ipv6->GetNAddresses (ifaceIdx); i < l; ++i) {
            Ipv6InterfaceAddress addr = m_ipv6->GetAddress (ifaceIdx, i);
            NS_LOG_INFO("addr[" << i << "]: " << addr.GetAddress() << addr.GetPrefix());
        }
        if (ifaceData.IsState(InterfaceState::DOWN)) continue;

        // Link Typeが2の場合、LA-bitが立ったプレフィクスだけを追加する
        bool onlyLocal = !(
            ifaceData.GetType() == InterfaceType::P2P ||
            ifaceData.GetType() == InterfaceType::VIRTUAL
        );

        // リンクがP2Mの場合かLOOPBACKであるとき、グローバルスコープIPv6アドレスが存在するなら、128ビット、距離0で追加する(LA-bitも？)
        bool perfectPrefix = (
            ifaceData.GetType() == InterfaceType::P2M ||
            ifaceData.IsState(InterfaceState::LOOPBACK)
        );

        // それ以外では、グローバルなプレフィクスを全て追加する。メトリックはインターフェイスに従う。
        uint16_t metric = CalcMetricForInterface(ifaceIdx);
        uint8_t prefixLength, options;
        for (uint32_t i = 0, l = m_ipv6->GetNAddresses (ifaceIdx); i < l; ++i) {
            Ipv6InterfaceAddress addr = m_ipv6->GetAddress (ifaceIdx, i);
            if (
                addr.GetAddress () != Ipv6Address () &&
                addr.GetPrefix () != Ipv6Prefix () &&
                addr.GetScope() == Ipv6InterfaceAddress::GLOBAL
            ) {
                NS_LOG_INFO("add prefix: " << addr.GetAddress() << addr.GetPrefix());
                prefixLength = addr.GetPrefix().GetPrefixLength();
                options = 0x0; // リンクに存在する唯一のルータの場合は0x2(LA-bit)とし、プレフィクス長も128bitにする（ホストアドレス扱い）
                body.AddPrefix(addr.GetAddress(), prefixLength, metric, options);
            }
        }

        // エリアをまたいで設定された仮想リンクが存在するなら、その相手方のアドレスを128ビット、距離0で追加する
        // FIXME: 書く
    }

    NS_LOG_INFO("Intra-Area-Prefix-LSA for #" << m_routerId << " result: " << *lsa);

    bool updateFlag = true;
    if (m_lsdb.Has(id)) {
        updateFlag = !(*m_lsdb.Get(id)->GetBody() == *lsa->GetBody());
    } else {
        RegisterToLSDB(lsa);
    }
    UpdateLSACaches(lsa);
    RemoveFromAllRxmtList(id);
    AppendToRxmtList(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
    if (updateFlag) {
        if (m_tableUpdateReducible) {
            m_tableUpdateRequired = true;
        } else {
            CalcRoutingTable();
        }
    }
}

void Ipv6OspfRouting::OriginateRouterSpecificLSAs (uint32_t ifaceIdx, bool forceRefresh) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx);
    m_tableUpdateReducible = true;
    m_tableUpdateRequired = false;
    OriginateRouterLSA(forceRefresh);
    OriginateLinkLSA(ifaceIdx, forceRefresh);
    OriginateIntraAreaPrefixLSA(forceRefresh);
    if (m_tableUpdateRequired) {
        CalcRoutingTable();
    }
    m_tableUpdateRequired = false;
    m_tableUpdateReducible = false;
}

uint16_t Ipv6OspfRouting::CalcMetricForInterface (uint32_t ifaceIdx) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx);
    Ptr<NetDevice> netDevice = m_ipv6->GetNetDevice(ifaceIdx);
    if (netDevice->IsPointToPoint()) {
        Ptr<PointToPointNetDevice> p2pNetDev = DynamicCast<PointToPointNetDevice>(netDevice);
        DataRateValue dataRateValue;
        p2pNetDev->GetAttribute("DataRate", dataRateValue);
        return (uint16_t)(1e8 / dataRateValue.Get().GetBitRate());
    } else {
        NS_LOG_ERROR("unknown interface device type:" << netDevice);
    }
    return 1;
}

void Ipv6OspfRouting::ReceiveHelloPacket(uint32_t ifaceIdx, Ipv6Address srcAddr, Ptr<Packet> packet) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << srcAddr);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFHello helloPacket;
    packet->RemoveHeader(helloPacket);

    RouterId neighborRouterId = helloPacket.GetRouterId();

    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);

    bool isFirstHello = !neighData.IsInitialized();
    if (isFirstHello) {
        neighData.MinimalInitialize(srcAddr, helloPacket);
    }

    Timer& inactivityTimer = neighData.GetInactivityTimer();
    inactivityTimer.Cancel();
    inactivityTimer.SetFunction(&Ipv6OspfRouting::NotifyNeighborEvent, this);
    inactivityTimer.SetArguments(ifaceIdx, neighborRouterId, NeighborEvent::INACTIVE);
    inactivityTimer.SetDelay(ifaceData.GetRouterDeadInterval());

    // Helloを送ってきたネイバーステートマシンにHelloReceived発行
    NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::HELLO_RECEIVED);

    // ネイバーリストに送信者ネイバーがいるならネイバーステートマシンに2-WayReceived発行
    // そうでなければネイバーステートマシンに1-WayReceived発行
    if (ifaceData.IsKnownNeighbor(neighborRouterId)) {
        NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::TWOWAY_RECEIVED);
    } else {
        neighData.Initialize(srcAddr, helloPacket);
        NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::ONEWAY_RECEIVED);
        return;
    }

    bool flagNeighChange = false, flagBackupSeen = false;

    // router priority値が過去のものと異なればインターフェースステートマシンにNeighborChange発行
    if (!isFirstHello && helloPacket.GetRouterPriority() != neighData.GetRouterPriority()) {
        NotifyInterfaceEvent(ifaceIdx, InterfaceEvent::NEIGH_CHANGE);
    }

    // HelloパケットのDRフィールドにネイバー自身が、BDRが0.0.0.0で、かつ受け取ったインターフェースがWaitingのとき
    // インターフェースステートマシンはBackupSeenイベントを非同期で発行する
    // そうでなく、ネイバーがそれまで自身をDRに指定していないか、DRの指定が今までなかった場合、
    // インターフェースステートマシンはNeighborChangeイベントを非同期で発行する
    if (helloPacket.GetDesignatedRouter() == neighborRouterId) {
        if (
            helloPacket.GetBackupDesignatedRouter() == 0 &&
            ifaceData.IsState(InterfaceState::WAITING)
        ) {
            flagBackupSeen = true;
        } else if (
            neighData.GetDesignatedRouter() != neighborRouterId ||
            neighData.GetDesignatedRouter() == 0
        ) {
            flagNeighChange = true;
        }
    }

    // HelloパケットのBDRフィールドにネイバー自身が指定されていて、かつ受け取ったインターフェースがWaitingのとき
    // インターフェースステートマシンはBackupSeenイベントを非同期で発行する
    // そうでなく、ネイバーがそれまで自身をBDRに指定していないか、BDRの指定が今までなかった場合、
    // インターフェースステートマシンはNeighborChangeイベントを非同期で発行する
    if (helloPacket.GetBackupDesignatedRouter() == neighborRouterId) {
        if (ifaceData.IsState(InterfaceState::WAITING)) {
            flagBackupSeen = true;
        } else if (
            neighData.GetBackupDesignatedRouter() != neighborRouterId ||
            neighData.GetBackupDesignatedRouter() == 0
        ) {
            flagNeighChange = true;
        }
    }

    uint32_t ifaceIdCache = neighData.GetInterfaceId();
    neighData.Initialize(srcAddr, helloPacket);
    Simulator::ScheduleNow(&Ipv6OspfRouting::SendLinkStateUpdatePacketEntryPoint, this);

    if (ifaceIdCache != neighData.GetInterfaceId()) {
        OriginateRouterLSA();
    }

    // TODO: ちゃんと非同期実行にする
    // 結局中身が未実装なので不具合があるかどうかが不明
    if (flagBackupSeen) {
        NotifyInterfaceEvent(ifaceIdx, InterfaceEvent::BACKUP_SEEN);
    }
    if (flagNeighChange) {
        NotifyInterfaceEvent(ifaceIdx, InterfaceEvent::NEIGH_CHANGE);
    }
}

void Ipv6OspfRouting::ReceiveDatabaseDescriptionPacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
    NS_LOG_FUNCTION(m_routerId << packet);
    NS_LOG_INFO("routerID: " << m_routerId << ", ifaceIdx: " << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFDatabaseDescription ddPacket;
    packet->RemoveHeader(ddPacket);

    NS_LOG_INFO("received: " << ddPacket);

    RouterId neighborRouterId = ddPacket.GetRouterId();
    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);

    // lastReceivedDdPacketに保存する。このときLSAHeader部は本来不要
    neighData.SetLastReceivedDD(ddPacket);

    // InterfaceMTUがこのルータの受け取り可能サイズを超えている場合、断片化しているのでreject
    if (ddPacket.GetMtu() > m_ipv6->GetMtu(ifaceIdx)) {
        NS_LOG_INFO("断片化しているようです!!!");
        return;
    }

    NS_LOG_INFO("NeighborState: " << ToString(neighData.GetState()));

    switch (neighData.GetState()) {
    case NeighborState::DOWN:
    case NeighborState::ATTEMPT:
    case NeighborState::TWOWAY:
        return;

    case NeighborState::INIT: {
        NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::TWOWAY_RECEIVED);
        if (!neighData.IsState(NeighborState::EXSTART)) {
            NS_LOG_LOGIC("TWOWAY_RECEIVEDした結果EXSTARTにならなかったので中断");
            return;
        }
        // fall through
    }
    case NeighborState::EXSTART: {
        if (ddPacket.IsNegotiation() && m_routerId < neighborRouterId) {
            neighData.SetAsSlave();
            neighData.SetSequenceNumber(ddPacket.GetSequenceNumber());
            NS_LOG_LOGIC("Exchange start: (" << m_routerId << "の視点): master - " << neighborRouterId << ", slave - " << m_routerId);
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::NEGOT_DONE);
            return;
        } else if (
            !(ddPacket.GetInitFlag() || ddPacket.GetMasterFlag()) &&
            ddPacket.GetSequenceNumber () == neighData.GetSequenceNumber() &&
            m_routerId > neighborRouterId
        ) {
            neighData.SetAsMaster();
            NS_LOG_LOGIC("Exchange start: (" << m_routerId << "の視点): master - " << m_routerId << ", slave - " << neighborRouterId);
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::NEGOT_DONE);
            neighData.IncrementSequenceNumber();
            return;
        }
        NS_LOG_LOGIC("EXSTARTだが何も条件を満たしていなかった");
        NS_LOG_LOGIC("Slave条件: ddPacket.IsNegotiation() -> " << (ddPacket.IsNegotiation() ? "true" : "false") << " && m_routerId < neighborRouterId -> " << (m_routerId < neighborRouterId ? "true" : "false"));
        NS_LOG_LOGIC("Master条件: !(ddPacket.GetInitFlag() || ddPacket.GetMasterFlag()) -> " << (!(ddPacket.GetInitFlag() || ddPacket.GetMasterFlag()) ? "true" : "false")
            << " && ddPacket.GetSequenceNumber () == neighData.GetSequenceNumber() -> " << ddPacket.GetSequenceNumber () << " == " << neighData.GetSequenceNumber () << (ddPacket.GetSequenceNumber () == neighData.GetSequenceNumber() ? "true" : "false")
            << " && m_routerId > neighborRouterId -> " << (m_routerId > neighborRouterId ? "true": "false"));
        return;
    }
    case NeighborState::EXCHANGE: {
        OSPFDatabaseDescription& lastPacket = neighData.GetLastPacket();
        if (
            ddPacket.GetMasterFlag() != lastPacket.GetMasterFlag() ||
            ddPacket.GetInitFlag() ||
            ddPacket.GetOptions() != lastPacket.GetOptions() || (
                neighData.IsMaster() &&
                ddPacket.GetSequenceNumber() != neighData.GetSequenceNumber()
            ) || (
                neighData.IsSlave() &&
                ddPacket.GetSequenceNumber() <= neighData.GetSequenceNumber()
            )
        ) {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::SEQ_NUM_MISMATCH);
            return;
        }
        break; // accepted
    }
    case NeighborState::LOADING:
    case NeighborState::FULL: {
        OSPFDatabaseDescription& lastPacket = neighData.GetLastPacket();
        if (ddPacket.GetOptions() != lastPacket.GetOptions()) {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::SEQ_NUM_MISMATCH);
            return;
        }
    }
    } // switch

    // accepted
    NS_LOG_LOGIC("Database Description accepted: " << m_routerId);

    /*
    もしlsTypeが未知またはAS-external-LSA(LS type = 5)でかつ相手がstub areaに属するネイバーなら、SeqNumMismatchを発行して処理をやめる。
    */
    // TODO: LS Typeの確認
    OSPFLinkStateIdentifier identifier;
    for (auto lsaHeader : ddPacket.GetLSAHeaders()) {
        identifier = lsaHeader->CreateIdentifier();
        if (m_lsdb.Has(identifier)) {
            Ptr<OSPFLSA> storedLsa = m_lsdb.Get(identifier);
            if (lsaHeader->IsMoreRecentThan(*storedLsa->GetHeader())) {
                neighData.AddRequestList(lsaHeader);
            }
        } else {
            neighData.AddRequestList(lsaHeader);
        }
    }

    if (neighData.IsMaster()) {
        neighData.IncrementSequenceNumber();
        if (!ddPacket.GetMoreFlag()) {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::EXCHANGE_DONE);
        } else {
            Simulator::ScheduleNow(&Ipv6OspfRouting::SendDatabaseDescriptionPacket, this, ifaceIdx, neighborRouterId, false);
        }
    } else { // Slave
        neighData.SetSequenceNumber(ddPacket.GetSequenceNumber());
        Simulator::ScheduleNow(&Ipv6OspfRouting::SendDatabaseDescriptionPacket, this, ifaceIdx, neighborRouterId, false);
        if (!ddPacket.GetMoreFlag()) {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::EXCHANGE_DONE);
            Simulator::Schedule(ifaceData.GetRouterDeadInterval(), &NeighborData::ClearLastPacket, &neighData);
            OriginateRouterLSA(true);
        }
    }
}

void Ipv6OspfRouting::ReceiveLinkStateRequestPacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFLinkStateRequest lsrPacket;
    packet->RemoveHeader(lsrPacket);

    NS_LOG_INFO("received: " << lsrPacket);

    RouterId neighborRouterId = lsrPacket.GetRouterId();
    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);
    /*
    ネイバーがExchange, Loading, Fullのときaccepted、それ以外はignored
    このパケットで指定されたLSAはルータのデータベースにあるはず。LSUにコピーしてネイバーに送る。
    linkStateRxmtListに入れてはいけない。
    もしLSAがDBに見つからなかったり、何か失敗したら、BadLSReqを発行する。
    */

    if (!(
        neighData.IsState(NeighborState::EXCHANGE) ||
        neighData.IsState(NeighborState::LOADING) ||
        neighData.IsState(NeighborState::FULL)
    )) {
        NS_LOG_LOGIC("ReceiveLinkStateRequestPacket - ignored by state");
        return;
    }

    std::vector<Ptr<OSPFLSA> > lsas;
    for (const OSPFLinkStateIdentifier& identifier : lsrPacket.GetLinkStateIdentifiers()) {
        if (m_lsdb.Has(identifier)) {
            lsas.push_back(m_lsdb.Get(identifier));
        } else {
            NS_LOG_LOGIC("ReceiveLinkStateRequestPacket - emit BAD_LS_REQ");
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::BAD_LS_REQ);
            return;
        }
    }
    NS_LOG_LOGIC("ReceiveLinkStateRequestPacket - Updateがスケジュールされました");
    Simulator::ScheduleNow(&Ipv6OspfRouting::SendLinkStateUpdatePacketDirect, this, ifaceIdx, lsas, neighborRouterId);
}

void Ipv6OspfRouting::ReceiveLinkStateUpdatePacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFLinkStateUpdate lsuPacket;
    packet->RemoveHeader(lsuPacket);

    NS_LOG_INFO("received: " << lsuPacket);

    RouterId neighborRouterId = lsuPacket.GetRouterId();
    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);

    // DRやBDRである可能性を考慮していません！

    if (neighData.GetState() < NeighborState::EXCHANGE) {
        NS_LOG_INFO("ReceiveLinkStateUpdatePacket(" << m_routerId << ", " << ifaceIdx << ") - 拒否しました " << ToString(neighData.GetState()));
        return;
    }

    std::vector<Ptr<OSPFLSAHeader> > lsasForDelayedAck;
    bool recalcRoutingTableRequired = false;

    for (auto received : lsuPacket.GetLSAs()) {
        OSPFLinkStateIdentifier identifier = received->GetIdentifier();
        bool hasInLSDB = m_lsdb.Has(identifier);
        bool isMoreRecent = (
            hasInLSDB &&
            received->GetHeader()->IsMoreRecentThan(*m_lsdb.Get(identifier)->GetHeader())
        );
        bool isSameInstance = (
            hasInLSDB &&
            !isMoreRecent &&
            received->GetHeader()->IsSameInstance(*m_lsdb.Get(identifier)->GetHeader())
        );
        bool isSelfOriginated = identifier.IsOriginatedBy(m_routerId, m_rtrIfaceId_set);

        // 1,2,3無視
        // 4
        if (
            received->GetHeader()->GetAge() == g_maxAge &&
            m_lsdb.Has(identifier) == 0 &&
            !ifaceData.HasExchangingNeighbor()
        ) {
            // 13.5 direct ack
            SendLinkStateAckPacket(ifaceIdx, received->GetHeader(), neighborRouterId);
            NS_LOG_INFO("dispose && ack sent:" << received);
            break; // dispose
        }

        // 5
        if (!hasInLSDB || isMoreRecent) {
            if (isMoreRecent) {
                NS_LOG_LOGIC("LSDBのものより新しいものを受け取りました");
            } else {
                NS_LOG_LOGIC("LSDBにまだないものを受け取りました");
            }
            bool isFlooded = false;
            // 5.a

            if (
                hasInLSDB &&
                !isSelfOriginated &&
                !m_lsdb.IsElapsedMinLsArrival(identifier)
            ) {
                NS_LOG_LOGIC("ReceiveLinkStateUpdatePacket(" << m_routerId << ", " << ifaceIdx << ") - 拒否されました: まだMinLSArrival経過していません");
                NS_LOG_INFO("rejected:" << received);
                continue; // dispose
            } else {
                // 5.b
                // 必要なネイバーに送る。DRであるときは個別に送り返す
                NS_LOG_LOGIC("ReceiveLinkStateUpdatePacket(" << m_routerId << ", " << ifaceIdx << ") - 受け付けました");
                AppendToRxmtList(received, ifaceIdx, neighborRouterId, true);
                isFlooded = true;
            }

            if (isSelfOriginated) {
                // 13.4を見よ
                // 受け取ったLSAがより新しい場合、シーケンス番号をそれより進めてインスタンス再生成し、flooding
                if (isMoreRecent) {
                    NS_LOG_WARN("received lsa is self originated and more recent than stored");
                    // TODO: タイプごとにちゃんと生成する
                    Ptr<OSPFLSA> stored = m_lsdb.Get(identifier);
                    stored->GetHeader()->SetAge(0);
                    stored->GetHeader()->SetSequenceNumber(
                        received->GetHeader()->GetSequenceNumber() + 1
                    );
                    // flooding
                    AppendToRxmtList(stored, ifaceIdx, neighborRouterId, true);
                    isFlooded = true;
                }

                // LSAを発信したくない場合は受け取ったLSAのLS AgeをMaxAgeにして再フラッディングする
                // 1）LSAがsummary-LSAまたはAS-external-LSAであり、ルータは宛先への（広告可能な）ルートを持たない
                // 2）network-LSAだが、ルータはもうDRでない
                // 3）Link State IDがルータ自身のInterface IDの1つだが、広告ルータとこのルータのルータIDが等しくない
                if (false/* FIXME: やって */) {
                    received->GetHeader()->SetAge(g_maxAge);
                    // flooding
                    AppendToRxmtList(received, ifaceIdx, neighborRouterId);
                }
            }

            // 5.c
            RemoveFromAllRxmtList(identifier);

            // 5.d
            NS_LOG_LOGIC("新しいLSAがインストールされます！ - インストールされるLSA: " << *received);
            RegisterToLSDB(received);
            // 13.2を見よ
            recalcRoutingTableRequired = true;

            // 5.e , 13.5を見よ
            if (!isFlooded) {
                // DRやBDRやでなんやかやあるらしい
                // delayed ack
                lsasForDelayedAck.push_back(received->GetHeader());
            }

            // 5.f
            // 上方へ移動した
        }

        // 8
        if (hasInLSDB && !(isMoreRecent || isSameInstance)) {

            // 6
            if (neighData.HasInRequestList(identifier)) {
                NS_LOG_LOGIC("ReceiveLinkStateUpdatePacket(" << m_routerId << ", " << ifaceIdx << "), neighbor #" << neighData.GetRouterId() << " - emit BAD_LS_REQ - リクエストリストに入れている間に流れてきた");
                NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::BAD_LS_REQ);
                break;
            }
            
            // 受け取ったLSAがDB内のものより古い場合
            if (m_lsdb.Get(identifier)->IsDeprecatedInstance()) {
                NS_LOG_INFO("DBにあるものはすでにdeprecatedだった");
                continue;
            } else if (GetLastLSUSentTime() + g_minLsArrival <= ns3::Now()) {
                // LSUでDB内のLSAを直接送り返す
                NS_LOG_INFO("受け取ったものよりDBにあるものが新しいので送り返す");
                DirectAppendToRxmtList(m_lsdb.Get(identifier), ifaceIdx, neighborRouterId);
            }
        }


        // 7
        if (neighData.HasInRxmtList(identifier)) {
            if (isSameInstance) {
                NS_LOG_INFO("ReceiveLinkStateUpdatePacket(" << m_routerId << ", " << ifaceIdx << "), neighbor #" << neighData.GetRouterId() << " - 暗黙の確認応答によりRxmt Listから削除します: " << identifier);
                neighData.RemoveFromRxmtList(identifier);
                // もしBDRなら処理があるので、実装する場合は 13. を見ること
            } else {
                NS_LOG_INFO("ReceiveLinkStateUpdatePacket(" << m_routerId << ", " << ifaceIdx << "), neighbor #" << neighData.GetRouterId() << " - 暗黙の確認応答とは判定されませんでした: " << identifier);
            }
        } else {
            if (isSameInstance) {
                // direct ack
                SendLinkStateAckPacket(ifaceIdx, received->GetHeader(), neighborRouterId);
            }
        }
    }
    if (recalcRoutingTableRequired) {
        CalcRoutingTable ();
    }
    SendLinkStateAckPacket(ifaceIdx, lsasForDelayedAck, neighborRouterId);
}

std::ostream& operator<< (std::ostream& os, const OSPFLinkStateIdentifier& id) {
    os << "OSPFLinkStateIdentifier [ ";
    os << "type: " << id.m_type << ", ";
    os << "id: " << id.m_id << ", ";
    os << "advRtr: " << id.m_advRtr << " ]";
    return os;
}

void Ipv6OspfRouting::DirectAppendToRxmtList (Ptr<OSPFLSA> lsa, uint32_t ifaceIdx, RouterId neighborRouterId, bool sendAsap) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << neighborRouterId << *lsa);
    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    NeighborData& neighData = ifaceData.GetNeighbor(neighborRouterId);
    auto lsHdr = lsa->GetHeader();
    auto identifier = lsa->GetIdentifier();
    // if (sendAsap) {
    //     SendLinkStateUpdatePacketDirectAsap(ifaceIdx, lsa, neighborRouterId);
    // } else {
    //     neighData.AddRxmtList(lsa);
    // }

    // 1.a
    if (neighData.GetState() < NeighborState::EXCHANGE) {
        NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << neighborRouterId << ": 状態が不正 " << neighData.GetState());
        // return; // next neighbor
    }

    // 1.b
    if (!neighData.IsState(NeighborState::FULL)) {
        if (neighData.HasInRequestList(identifier)) {
            Ptr<OSPFLSAHeader> lsHdrInReqList = neighData.GetFromRequestList(identifier);
            if (lsHdrInReqList->IsMoreRecentThan(*lsHdr)) {
                NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << neighborRouterId << ": ReqListにあるものの方が新しい");
                // return; // next neighbor
            }

            NS_LOG_LOGIC("remove from request list - interface " << ifaceIdx << ", neighbor " << neighborRouterId);
            neighData.RemoveFromRequestList(identifier);

            if (neighData.IsRequestListEmpty()) {
                NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::LOADING_DONE);
            }

            if (lsHdrInReqList->IsSameInstance(*lsHdr)) {
                NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << neighborRouterId << ": ReqListにあるものと同じ");
                // return; // next neighbor
            }

            NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << neighborRouterId << ": ReqListにあるものは古い");
        }
    }

    // 1.c
    // if (lsHdr->GetAdvertisingRouter() == neighData.GetRouterId()) {
    //     NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << neighborRouterId << ": 発行ルータがこのネイバーだった");
    //     return; // next neighbor
    // }

    // 1.d
    if (sendAsap) {
        SendLinkStateUpdatePacketDirectAsap(ifaceIdx, lsa, neighborRouterId);
    } else {
        bool ret = neighData.AddRxmtList(lsa);
        if(!ret) NS_LOG_INFO("AddRxmtList failed");
    }
}

void Ipv6OspfRouting::AppendToRxmtList (Ptr<OSPFLSA> lsa, uint32_t receivedIfaceIdx, RouterId senderRouterId, bool sendAsap) {
    NS_LOG_FUNCTION(m_routerId << receivedIfaceIdx << senderRouterId << lsa << (sendAsap ? "asap" : ""));
    Ptr<OSPFLSAHeader> lsHdr = lsa->GetHeader();
    OSPFLinkStateIdentifier identifier = lsa->GetIdentifier();
    bool isAlreadyAddedToRxmtList = false;
    uint32_t targetArea = m_interfaces[receivedIfaceIdx].GetAreaId();

    for (auto ifaceIdx : m_rtrIfaceId_set) {
        InterfaceData& ifaceData = m_interfaces[ifaceIdx];
        NS_LOG_LOGIC("AppendToRxmtList("<<m_routerId<<", "<<receivedIfaceIdx<<") from "<<senderRouterId<<": iface #" << ifaceIdx << "");
        if (!ifaceData.IsActive()) {
            NS_LOG_LOGIC("reject interface " << ifaceIdx << ": inactive");
            continue;
        }
        if (
            lsHdr->GetType() != OSPF_LSA_TYPE_AS_EXTERNAL &&
            ifaceData.GetAreaId() != targetArea
        ) {
            NS_LOG_LOGIC("reject interface " << ifaceIdx << ": AS-external && area unmatched");
            continue; // next iface
        }

        // https://tools.ietf.org/html/rfc5340#section-4.5.2
        if (lsHdr->IsAreaScope()) {
            if (ifaceData.GetAreaId() != targetArea) {
                NS_LOG_LOGIC("reject interface " << ifaceIdx << ": area unmatched");
                continue; // next iface
            }
        }

        if (lsHdr->IsLinkLocalScope()) {
            if (lsa->GetBody<OSPFLinkLSABody>()) {
                OSPFLinkLSABody& body = *lsa->GetBody<OSPFLinkLSABody>();
                bool onValidLink = body.GetLinkLocalAddress() == ifaceData.GetAddress();
                if (!onValidLink) {
                    for (auto& kv : ifaceData.GetNeighbors()) {
                        // 受け取ったやつ
                        if (body.GetLinkLocalAddress() == kv.second.GetAddress()) {
                            onValidLink = true;
                            break;
                        }
                    }
                }
                if (!onValidLink) {
                    NS_LOG_LOGIC("reject interface " << ifaceIdx << ": link-local");
                    continue; // next iface
                }
            }
        }

        isAlreadyAddedToRxmtList = false;
        // 1
        NS_LOG_LOGIC("AppendToRxmtList("<<m_routerId<<", "<<receivedIfaceIdx<<") from "<<senderRouterId<<": iface #" << ifaceIdx << " - neighbors " << ifaceData.CountNeighbors());
        for (auto& kv : ifaceData.GetNeighbors()) {
            NeighborData& neighData = kv.second;
            NS_LOG_LOGIC("AppendToRxmtList("<<m_routerId<<", "<<receivedIfaceIdx<<") from "<<senderRouterId<<": iface #" << ifaceIdx << " - neigh #" << kv.first);
            // 1.a
            if (neighData.GetState() < NeighborState::EXCHANGE) {
                NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << kv.first << ": 状態が不正 " << neighData.GetState());
                continue; // next neighbor
            }
            // 1.b
            if (!neighData.IsState(NeighborState::FULL)) {
                if (neighData.HasInRequestList(identifier)) {
                    Ptr<OSPFLSAHeader> lsHdrInReqList = neighData.GetFromRequestList(identifier);
                    if (lsHdrInReqList->IsMoreRecentThan(*lsHdr)) {
                        NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << kv.first << ": ReqListにあるものの方が新しい");
                        continue; // next neighbor
                    }

                    NS_LOG_LOGIC("remove from request list - interface " << ifaceIdx << ", neighbor " << kv.first);
                    neighData.RemoveFromRequestList(identifier);

                    if (neighData.IsRequestListEmpty()) {
                        NotifyNeighborEvent(ifaceIdx, kv.first, NeighborEvent::LOADING_DONE);
                    }

                    if (lsHdrInReqList->IsSameInstance(*lsHdr)) {
                        NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << kv.first << ": ReqListにあるものと同じ");
                        continue; // next neighbor
                    }

                    NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << kv.first << ": ReqListにあるものは古い");
                }
            }
            // 1.c
            if (lsHdr->GetAdvertisingRouter() == neighData.GetRouterId()) {
                NS_LOG_LOGIC("reject interface " << ifaceIdx << ", neighbor " << kv.first << ": 発行ルータがこのネイバーだった");
                continue; // next neighbor
            }

            // 1.d
            
            if (sendAsap) {
                SendLinkStateUpdatePacketDirectAsap(ifaceIdx, lsa, kv.first);
            } else {
                bool ret = neighData.AddRxmtList(lsa);
                if(!ret) NS_LOG_INFO("AddRxmtList failed");
            }
            isAlreadyAddedToRxmtList = true;
            NS_LOG_LOGIC("accepted on interface " << ifaceIdx << ", neighbor " << kv.first);
        }

        // 2
        if (isAlreadyAddedToRxmtList) {
            NS_LOG_LOGIC("accept interface " << ifaceIdx << ": ネイバーのRxmtListに入れたので終わり");
            continue; // next iface
        }

        // 3
        if (
            ifaceData.IsIndex(receivedIfaceIdx) && (
                ifaceData.GetDesignatedRouter() == senderRouterId ||
                ifaceData.GetBackupDesignatedRouter() == senderRouterId
            )
        ) {
            NS_LOG_LOGIC("reject interface " << ifaceIdx << ": DRから受け取ったので終わり");
            continue; // next iface
        }

        // 4
        if (
            ifaceData.IsIndex(receivedIfaceIdx) &&
            ifaceData.IsState(InterfaceState::BACKUP)
        ) {
            NS_LOG_LOGIC("reject interface " << ifaceIdx << ": 自分がBDRなので終わり");
            continue;
        }

        // 5
        NS_LOG_LOGIC("accepted on interface " << ifaceIdx);
        if (sendAsap) {
            SendLinkStateUpdatePacketDirectAsap(ifaceIdx, lsa, 0);
        } else {
            ifaceData.AddRxmtList(lsa);
        }
    }
}

Time& Ipv6OspfRouting::GetLastLSUSentTime () {
    return m_lastLsuSendTime;
}

void Ipv6OspfRouting::ReceiveLinkStateAckPacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];
    if (!ifaceData.IsActive()) return;

    OSPFLinkStateAck lsaPacket;
    packet->RemoveHeader(lsaPacket);

    NS_LOG_INFO("received: " << lsaPacket);

    RouterId neighborRouterId = lsaPacket.GetRouterId();
    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);
    /*
    細かい送信条件はFlooding Procedureを参照
    ネイバーがExchange未満の状態なら破棄する。
    各LSAHeaderに対して、
    - このネイバーに対するlinkStateRxmtListにinstanceがあるか？ なければ次へ
        - 同じinstanceならrxmtListから削除して次へ
        - そうでなければ、ログに残して次へ
    */
    if (neighData.GetState() < NeighborState::EXCHANGE) {
        NS_LOG_LOGIC("rejected: " << ToString(neighData.GetState()));
        return;
    }
    
    std::vector<Ptr<OSPFLSA> >& rxmtList = neighData.GetRxmtList();
    std::vector<Ptr<OSPFLSAHeader> >& lsaList = lsaPacket.GetLSAHeaders();
    for (auto ackedLsaHdr : lsaPacket.GetLSAHeaders()) {
        for (auto iter = rxmtList.begin(); iter != rxmtList.end(); ) {
            if (ackedLsaHdr->IsSameInstance(*((*iter)->GetHeader()))) {
                NS_LOG_LOGIC("Remove from rxmt list: " << ackedLsaHdr);
                iter = rxmtList.erase(iter);
                break;
            } else {
                ++iter;
            }
        }
    }
    // for (
    //     auto it = rxmtList.begin();
    //     it != rxmtList.end();
    //     // nop
    // ) {
    //     int i = 0, l = lsaList.size();
    //     while (i < l && !lsaList[i].IsSameIdentity(*(it->GetHeader()))) ++i;
    //     if (i != l) {
    //         if (lsaList[i].IsSameInstance(*(it->GetHeader()))) {
    //             NS_LOG_LOGIC("Remove from rxmt list: " << lsaList[i]);
    //             it = rxmtList.erase(it);
    //         } else {
    //             NS_LOG_WARN("BAD ACK: ");
    //         }
    //     } else {
    //         ++it;
    //     }
    // }
}

void Ipv6OspfRouting::RemoveFromAllRxmtList(OSPFLinkStateIdentifier &identifier) {
    NS_LOG_FUNCTION(m_routerId);
    if(m_lsdb.Has(identifier))
        NS_LOG_INFO("RemoveFromAllRxmtList - 持っているLSAです: " << identifier);
    else
        NS_LOG_INFO("RemoveFromAllRxmtList - 持っていないLSAです: " << identifier);

    for (InterfaceData& ifaceData : m_interfaces) {
        if (!ifaceData.IsActive()) continue;
        for (auto& kv : ifaceData.GetNeighbors()) {
            kv.second.RemoveFromRxmtList(identifier);
        }
    }
}

bool Ipv6OspfRouting::IsNeighborToBeAdjacent(uint32_t ifaceIdx, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << neighborRouterId);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];
    InterfaceType ifaceType = ifaceData.GetType();
    NS_LOG_INFO("interface type: " << ToString(ifaceType));

    // OSPFv2 10.4.  Whether to become adjacent
    // https://tools.ietf.org/html/rfc2328#page-96
    return (
        ifaceType     == InterfaceType::P2P ||
        ifaceType     == InterfaceType::P2M ||
        ifaceType     == InterfaceType::VIRTUAL ||
        m_routerId    == ifaceData.GetDesignatedRouter() ||
        m_routerId    == ifaceData.GetBackupDesignatedRouter() ||
        neighborRouterId  == ifaceData.GetDesignatedRouter() ||
        neighborRouterId  == ifaceData.GetBackupDesignatedRouter()
    );
}

void Ipv6OspfRouting::NotifyInterfaceEvent(uint32_t ifaceIdx, InterfaceEvent event) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << ToString(event));
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];
    InterfaceState beforeState = ifaceData.GetState();

    // OSPFv2 10.3 The Interface state machine
    // https://tools.ietf.org/html/rfc2328#page-72
    switch (event) {
    case InterfaceEvent::IF_UP: {
        if (ifaceData.IsState(InterfaceState::DOWN)) {

            if (
                ifaceData.IsType(InterfaceType::P2P) ||    
                ifaceData.IsType(InterfaceType::P2M) ||    
                ifaceData.IsType(InterfaceType::VIRTUAL)    
            ) {
                ifaceData.SetState(InterfaceState::P2P);
                OriginateRouterSpecificLSAs(ifaceIdx);
                break;
            }

            if (ifaceData.IsEligibleToDR()) {
                if (
                    ifaceData.IsType(InterfaceType::BROADCAST) ||
                    ifaceData.IsType(InterfaceType::NBMA)
                ) {
                    ifaceData.SetState(InterfaceState::WAITING);
                    OriginateRouterSpecificLSAs(ifaceIdx);
                    ifaceData.StartWaitTimer();
                }
            } else {
                ifaceData.SetState(InterfaceState::DR_OTHER);
                OriginateRouterSpecificLSAs(ifaceIdx);
                break;
            }


            if (ifaceData.IsType(InterfaceType::NBMA)) {
                NS_LOG_ERROR("NBMA connection is not supported");
                // TODO: このインターフェースの設定済みネイバーリストを検査(NBMAが未実装なのでこの状態にする)
                // 各ネイバーにStartイベントを発行
                break;
            }
        }
        break;
    }
    case InterfaceEvent::BACKUP_SEEN: // fall through
    case InterfaceEvent::WAIT_TIMER: {
        if (ifaceData.IsState(InterfaceState::WAITING)) {
            ifaceData.CalcDesignatedRouter();
        }
        break;
    }
    case InterfaceEvent::NEIGH_CHANGE: {
        if (
            ifaceData.IsState(InterfaceState::DR_OTHER) ||
            ifaceData.IsState(InterfaceState::DR) ||
            ifaceData.IsState(InterfaceState::BACKUP)
        ) {
            ifaceData.CalcDesignatedRouter();
        }
        break;
    }
    case InterfaceEvent::IF_DOWN: {
        ifaceData.SetState(InterfaceState::DOWN);
        OriginateRouterSpecificLSAs(ifaceIdx);
        ifaceData.ResetInstance();
        for (auto& kv : ifaceData.GetNeighbors()) {
            NotifyNeighborEvent(ifaceIdx, kv.first, NeighborEvent::KILL_NBR);
        }
        break;
    }
    case InterfaceEvent::LOOP_IND: {
        ifaceData.SetState(InterfaceState::LOOPBACK);
        OriginateRouterSpecificLSAs(ifaceIdx);
        ifaceData.ResetInstance();
        for (auto& kv : ifaceData.GetNeighbors()) {
            NotifyNeighborEvent(ifaceIdx, kv.first, NeighborEvent::KILL_NBR);
        }
        break;
    }
    case InterfaceEvent::UNLOOP_IND: {
        if (ifaceData.IsState(InterfaceState::LOOPBACK)) {
            ifaceData.SetState(InterfaceState::DOWN);
            OriginateRouterSpecificLSAs(ifaceIdx);
        }
        break;
    }
    } // switch

    NS_LOG_INFO("interface state mutation: " << ToString(beforeState) << " -> " << ToString(ifaceData.GetState()));
}

void Ipv6OspfRouting::NotifyNeighborEvent(uint32_t ifaceIdx, RouterId neighborRouterId, NeighborEvent event) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << neighborRouterId << ToString(event));
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];
    NeighborData &neighbor = ifaceData.GetNeighbor(neighborRouterId);

    NeighborState beforeState = neighbor.GetState();

    // OSPFv2 10.3 The Neighbor state machine
    // https://tools.ietf.org/html/rfc2328#page-89
    switch (event) {
    case NeighborEvent::START: {
        if (neighbor.IsState(NeighborState::DOWN)) {
            neighbor.SetState(NeighborState::ATTEMPT);
            // Hello送信はInterfaceUpで起動している
            neighbor.GetInactivityTimer().Schedule();
        }
        break;
    }
    case NeighborEvent::HELLO_RECEIVED: {
        if (neighbor.IsState(NeighborState::DOWN) ||
            neighbor.IsState(NeighborState::ATTEMPT))
        {
            neighbor.SetState(NeighborState::INIT);
        }
        neighbor.GetInactivityTimer().Schedule();
        break;
    }
    case NeighborEvent::TWOWAY_RECEIVED: {
        if (neighbor.IsState(NeighborState::INIT)) {
            if (IsNeighborToBeAdjacent(ifaceIdx, neighborRouterId)) {
                neighbor.SetState(NeighborState::EXSTART);
                neighbor.StartExchange();
                Simulator::ScheduleNow(&Ipv6OspfRouting::SendDatabaseDescriptionPacket, this, ifaceIdx, neighborRouterId, true);
            } else {
                neighbor.SetState(NeighborState::TWOWAY);
            }
        }
        break;
    }
    case NeighborEvent::NEGOT_DONE: {
        if (neighbor.IsState(NeighborState::EXSTART)) {
            neighbor.SetState(NeighborState::EXCHANGE);

            // FIXME: tooooo ad-hoc
            std::vector<Ptr<OSPFLSAHeader> > summarySeed, summaryList;
            std::vector<Ptr<OSPFLSA> > rxmt;
            m_lsdb.GetSummary(summarySeed, rxmt);

            for(auto hdr : summarySeed) {
                if(hdr->IsLinkLocalScope() && !ifaceData.IsKnownLinkLocalLSA(hdr->CreateIdentifier())) {
                    continue;
                }
                summaryList.push_back(hdr);
            }

            neighbor.SetSummaryList(summaryList);
            NS_LOG_LOGIC("Set Summary List: router " << m_routerId << ", nghRtrId " << neighborRouterId << ", size " << neighbor.GetSummaryListSize());
            for(auto maxAgedLsa : rxmt) {
                neighbor.AddRxmtList(maxAgedLsa);
            }
            Simulator::ScheduleNow(&Ipv6OspfRouting::SendDatabaseDescriptionPacket, this, ifaceIdx, neighborRouterId, false);
        }
        break;
    }
    case NeighborEvent::EXCHANGE_DONE: {
        if (neighbor.IsState(NeighborState::EXCHANGE)) {
            if (neighbor.IsRequestListEmpty()) {
                neighbor.SetState(NeighborState::FULL);
                OriginateRouterSpecificLSAs(ifaceIdx);
            } else {
                neighbor.SetState(NeighborState::LOADING);
                Simulator::ScheduleNow(&Ipv6OspfRouting::SendLinkStateRequestPacket, this, ifaceIdx, neighborRouterId);
            }
        }
        break;
    }
    case NeighborEvent::LOADING_DONE: {
        if (neighbor.IsState(NeighborState::LOADING)) {
            neighbor.SetState(NeighborState::FULL);
            OriginateRouterSpecificLSAs(ifaceIdx);
        }
        break;
    }
    case NeighborEvent::IS_ADJ_OK: {
        if (neighbor.IsState(NeighborState::TWOWAY)) {
            if (IsNeighborToBeAdjacent(ifaceIdx, neighborRouterId)) {
                neighbor.SetState(NeighborState::EXSTART);
                neighbor.StartExchange();
                Simulator::ScheduleNow(&Ipv6OspfRouting::SendDatabaseDescriptionPacket, this, ifaceIdx, neighborRouterId, true);
            }
            break;
        }
        if (neighbor.GetState() >= NeighborState::EXSTART) {
            if (!IsNeighborToBeAdjacent(ifaceIdx, neighborRouterId)) {
                neighbor.SetState(NeighborState::TWOWAY);
                neighbor.ClearList();
            }
        }
        break;
    }
    case NeighborEvent::BAD_LS_REQ: // fall through
    case NeighborEvent::SEQ_NUM_MISMATCH: {
        if (neighbor.GetState() >= NeighborState::EXCHANGE) {
            neighbor.SetState(NeighborState::EXSTART);
            neighbor.ClearList();
            neighbor.StartExchange();
            Simulator::ScheduleNow(&Ipv6OspfRouting::SendDatabaseDescriptionPacket, this, ifaceIdx, neighborRouterId, true);
        }
        break;
    }
    case NeighborEvent::KILL_NBR: // fall through
    case NeighborEvent::LL_DOWN: // fall through
    case NeighborEvent::INACTIVE: {
        neighbor.SetState(NeighborState::DOWN);
        neighbor.ClearList();
        break;
    }
    case NeighborEvent::ONEWAY_RECEIVED: {
        if (neighbor.GetState() >= NeighborState::TWOWAY) {
            neighbor.SetState(NeighborState::INIT);
            neighbor.ClearList();
        }
        break;
    }
    } // switch
    NS_LOG_INFO("neighbor state mutation ( " << m_routerId << ", " << neighborRouterId << " ): " << ToString(beforeState) << " -> " << ToString(neighbor.GetState()));
}

void Ipv6OspfRouting::SendHelloPacket(uint32_t ifaceIdx) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx);

    // 宛先はAllSPFRoutersでよい
    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    OSPFHello hello;

    hello.SetRouterId(m_routerId);
    hello.SetAreaId(ifaceData.GetAreaId());
    hello.SetInstanceId(0);
    hello.SetInterfaceId(ifaceData.GetInterfaceId());
    hello.SetRouterPriority(1);
    hello.SetOptions(0x13); // V6, E, R
    hello.SetHelloInterval(ifaceData.GetHelloInterval().ToInteger(Time::S));
    hello.SetRouterDeadInterval(ifaceData.GetRouterDeadInterval().ToInteger(Time::S));
    hello.SetDesignatedRouter(ifaceData.GetDesignatedRouter());
    hello.SetBackupDesignatedRouter(ifaceData.GetBackupDesignatedRouter());
    hello.SetNeighbors(ifaceData.GetActiveNeighbors());

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(hello);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    int res = socket->SendTo(packet, 0, Inet6SocketAddress(Ipv6OspfRouting::AllSPFRouters, PROTO_PORT));
    
    Simulator::ScheduleNow(&InterfaceData::ScheduleHello, &ifaceData);
}

void Ipv6OspfRouting::SendDatabaseDescriptionPacket(uint32_t ifaceIdx, RouterId neighborRouterId, bool isInit) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << neighborRouterId << (isInit ? "true" : "false"));

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    NeighborData& neighData = ifaceData.GetNeighbor(neighborRouterId);

    OSPFDatabaseDescription dd;

    dd.SetRouterId(m_routerId);
    dd.SetAreaId(ifaceData.GetAreaId());
    dd.SetInstanceId(0);
    dd.SetOptions(0x13); // V6, E, R
    uint32_t mtu = m_ipv6->GetMtu(ifaceIdx);
    dd.SetMtu(mtu); // TODO: 仮想リンクの場合は0にしなければならない
    dd.SetInitFlag(isInit);
    bool moreFlag = isInit || (
         (neighData.IsMaster() && neighData.HasMoreSummary()) ||
         (neighData.IsSlave() && neighData.GetLastPacket().GetMoreFlag())
    );
    dd.SetMoreFlag(moreFlag);
    dd.SetMasterFlag(isInit || neighData.IsMaster());
    uint32_t seqNum = neighData.GetSequenceNumber();
    dd.SetSequenceNumber(seqNum);
    if (!isInit && !neighData.GetLastPacket().GetInitFlag()) {
        dd.SetLSAHeaders(neighData.GetSummaryList(mtu - 40));
    }

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(dd);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    socket->SendTo(packet, 0, Inet6SocketAddress(neighData.GetAddress(), PROTO_PORT));
}
void Ipv6OspfRouting::SendLinkStateRequestPacket(uint32_t ifaceIdx, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << neighborRouterId);

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    NeighborData& neighData = ifaceData.GetNeighbor(neighborRouterId);

    if (neighData.GetRequestList().empty()) {
        NS_LOG_LOGIC("Request List for #" << m_routerId << " is empty");
        return;
    }

    OSPFLinkStateRequest lsr;

    lsr.SetRouterId(m_routerId);
    lsr.SetAreaId(ifaceData.GetAreaId());
    lsr.SetInstanceId(0);
    // lsr.SetOptions(0x13); // V6, E, R
    uint32_t mtu = m_ipv6->GetMtu(ifaceIdx);
    std::vector<OSPFLinkStateIdentifier> lsids;
    std::vector<Ptr<OSPFLSAHeader> > tmp = neighData.GetRequestList(mtu - 20);
    NS_LOG_LOGIC("Request List for #" << m_routerId << "size: " << neighData.GetRequestList().size() << ", partial size: " << tmp.size());
    for (auto lsHdr : tmp) {
        lsids.push_back(lsHdr->CreateIdentifier());
    }
    lsr.SetLinkStateIdentifiers(lsids);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(lsr);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    socket->SendTo(packet, 0, Inet6SocketAddress(neighData.GetAddress(), PROTO_PORT));

    if (!neighData.IsRequestListEmpty()) {
        Simulator::Schedule(ifaceData.GetRxmtInterval(), &Ipv6OspfRouting::SendLinkStateRequestPacket, this, ifaceIdx, neighborRouterId);
    }
}
void Ipv6OspfRouting::SendLinkStateAckPacket(uint32_t ifaceIdx, Ptr<OSPFLSAHeader> lsaHeader, RouterId neighborRouterId) {
    std::vector<Ptr<OSPFLSAHeader> > lsaHdrs;
    lsaHdrs.push_back(lsaHeader);
    Ipv6OspfRouting::SendLinkStateAckPacket(ifaceIdx, lsaHdrs, neighborRouterId);
}
void Ipv6OspfRouting::SendLinkStateUpdatePacketEntryPoint() {
    NS_LOG_FUNCTION(m_routerId);
    for (int i = 0, l = m_interfaces.size(); i < l; ++i) {
        if (!m_interfaces[i].IsActive()) {
            NS_LOG_LOGIC("inactive interface #" << i);
            continue;
        }
        SendLinkStateUpdatePacket(i);
    }
    Simulator::Schedule(Seconds(5.0), &Ipv6OspfRouting::SendLinkStateUpdatePacketEntryPoint, this);
    // Simulator::Schedule(m_interfaces[ifaceIdx].GetRxmtInterval(), &Ipv6OspfRouting::SendLinkStateUpdatePacket, this);
}
void Ipv6OspfRouting::SendLinkStateUpdatePacket(uint32_t ifaceIdx) {
    for (auto& kv : m_interfaces[ifaceIdx].GetNeighbors()) {
        if (
            kv.second.IsState(NeighborState::EXSTART) ||
            kv.second.IsState(NeighborState::EXCHANGE) ||
            kv.second.IsState(NeighborState::LOADING)
        ) {
            NS_LOG_LOGIC("state unmatched - interface #" << ifaceIdx << " is " << ToString(kv.second.GetState()));
            continue;
        }
        SendLinkStateUpdatePacket(ifaceIdx, kv.first);
    }
}
void Ipv6OspfRouting::SendLinkStateUpdatePacket(uint32_t ifaceIdx, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << neighborRouterId);

    if (neighborRouterId == 0) return;

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    NeighborData& neighData = ifaceData.GetNeighbor(neighborRouterId);
    if (neighData.GetRxmtList().empty()) {
        NS_LOG_LOGIC("Rxmt List for #" << m_routerId << " is empty");
        return;
    }

    Ipv6Address dstAddr = (
        ifaceData.GetType() == InterfaceType::P2P ?
            Ipv6OspfRouting::AllSPFRouters :
            ifaceData.GetNeighbor(neighborRouterId).GetAddress()
    );

    OSPFLinkStateUpdate lsu;
    lsu.SetRouterId(m_routerId);
    lsu.SetAreaId(ifaceData.GetAreaId());
    lsu.SetInstanceId(0);
    uint32_t mtu = m_ipv6->GetMtu(ifaceIdx);
    
    std::vector<Ptr<OSPFLSA> > tmp = neighData.GetRxmtList(mtu - 40);
    NS_LOG_LOGIC("Rxmt List for #" << m_routerId << " size: " << neighData.GetRxmtList().size() << ", partial size: " << tmp.size());
    NS_LOG_INFO("Rxmt List rtr: " << m_routerId << ", iface: " << ifaceIdx << ", nbr: " << neighborRouterId << " " << neighData.GetRxmtList());
    lsu.SetLSAs(tmp);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(lsu);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    socket->SendTo(packet, 0, Inet6SocketAddress(dstAddr, PROTO_PORT));
}
void Ipv6OspfRouting::SendLinkStateUpdatePacketDirectAsap(uint32_t ifaceIdx, Ptr<OSPFLSA> lsa, RouterId neighborRouterId) {
    std::vector<Ptr<OSPFLSA> > lsas = std::vector<Ptr<OSPFLSA> >();
    lsas.push_back(lsa);
    SendLinkStateUpdatePacketDirect(ifaceIdx, lsas, neighborRouterId);
}
void Ipv6OspfRouting::SendLinkStateUpdatePacketDirect(uint32_t ifaceIdx, std::vector<Ptr<OSPFLSA> >& lsas, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(m_routerId << ifaceIdx << neighborRouterId << lsas.size());

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    if (neighborRouterId == 0 && ifaceData.GetType() > InterfaceType::BROADCAST) {
        NS_LOG_WARN("Non-broadcast networkでは送信先ネイバーの指定が必須です");
        return;
    }

    Ipv6Address dstAddr = (
        (ifaceData.GetType() == InterfaceType::P2P || neighborRouterId == 0) ?
            Ipv6OspfRouting::AllSPFRouters :
            ifaceData.GetNeighbor(neighborRouterId).GetAddress()
    );

    OSPFLinkStateUpdate lsu;
    lsu.SetRouterId(m_routerId);
    lsu.SetAreaId(ifaceData.GetAreaId());
    lsu.SetInstanceId(0);

    std::vector<Ptr<OSPFLSA> > partialLsa;
    uint32_t mtu = m_ipv6->GetMtu(ifaceIdx);
    int32_t maxBytes = mtu - 40;
    while (!lsas.empty()) {
        maxBytes -= lsas.back()->GetSerializedSize();
        if (maxBytes <= 0) break;
        partialLsa.push_back(lsas.back());
        lsas.pop_back();
    }
    lsu.SetLSAs(partialLsa);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(lsu);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    socket->SendTo(packet, 0, Inet6SocketAddress(dstAddr, PROTO_PORT));

    if (lsas.size() > 0) {
        Simulator::ScheduleNow(&Ipv6OspfRouting::SendLinkStateUpdatePacketDirect, this, ifaceIdx, lsas, neighborRouterId);
    }
}
void Ipv6OspfRouting::SendLinkStateAckPacket(uint32_t ifaceIdx, std::vector<Ptr<OSPFLSAHeader> >& lsaHeaders, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(this << ifaceIdx << neighborRouterId);

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    Ipv6Address dstAddr = (
        ifaceData.GetType() == InterfaceType::P2P ?
            Ipv6OspfRouting::AllSPFRouters :
            ifaceData.GetNeighbor(neighborRouterId).GetAddress()
    );

    OSPFLinkStateAck lsack;
    lsack.SetRouterId(m_routerId);
    lsack.SetAreaId(ifaceData.GetAreaId());
    lsack.SetInstanceId(0);
    lsack.SetLSAHeaders(lsaHeaders);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(lsack);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    socket->SendTo(packet, 0, Inet6SocketAddress(dstAddr, PROTO_PORT));
}

Ptr<Ipv6Route> Ipv6OspfRouting::Lookup(Ipv6Address src, Ipv6Address dst, Ptr<NetDevice> interface) {
    NS_LOG_FUNCTION(m_routerId << src << dst << interface);

    Ptr<Ipv6Route> route = 0;

    Ipv6RoutingTableEntry* entry = new Ipv6RoutingTableEntry();
    if (m_routingTable.LookupRoute(dst, *entry)) {
        NS_LOG_LOGIC("Lookup succeeded");
        int32_t ifaceIdx = entry->GetInterface();
        NS_LOG_LOGIC("ifaceIdx: " << ifaceIdx);
        route = Create<Ipv6Route>();
        if (entry->GetGateway().IsAny()) {
            for (uint32_t i = 0, l = m_ipv6->GetNAddresses (ifaceIdx); i < l; ++i) {
                Ipv6InterfaceAddress addr = m_ipv6->GetAddress (ifaceIdx, i);
                if (
                    addr.GetAddress () != Ipv6Address () &&
                    addr.GetPrefix () != Ipv6Prefix () &&
                    addr.GetScope() == Ipv6InterfaceAddress::GLOBAL
                ) {
                    route->SetSource(m_ipv6->GetAddress(ifaceIdx, i).GetAddress());
                    break;
                }
            }
        } else if (entry->GetDest().IsAny()) {
            // default route
            NS_LOG_ERROR("!!!! Routing table contains default route");
        } else {
            route->SetSource(m_ipv6->SourceAddressSelection (ifaceIdx, entry->GetGateway()));
        }

        route->SetDestination(entry->GetDest());
        route->SetGateway(entry->GetGateway());
        route->SetOutputDevice(m_ipv6->GetNetDevice(ifaceIdx));
        return route;
    }

    NS_LOG_LOGIC("Lookup failed");

    return 0;
}

void Ipv6OspfRouting::CalcRoutingTable (bool recalcAll) {
    NS_LOG_FUNCTION(this << recalcAll);
    static uint16_t MAX_METRIC = 65535; // 2 ** 16 - 1
    static RouterId MAX_ROUTER_ID = 4294967295; // 2**32 - 1
    uint32_t routers = m_knownMaxRouterId + 1; // 0 is reserved
    NS_LOG_INFO("CalcRoutingTable - routerId: " << m_routerId << ", routers: " << routers);

    std::unordered_map<RouterId, std::unordered_map<RouterId, uint16_t> > table;
    uint16_t INFCOST = 65535;
    std::vector<uint16_t> costs(routers, 65535); // 経路長
    std::vector<RouterId> prevs(routers, 0); // 0は経路なしなので存在確認不要
    costs[m_routerId] = 0;

    NS_LOG_INFO("print entire LSDB");
    for (auto& kv : m_lsdb.GetTable()) {
        NS_LOG_INFO(" - " << *kv.second);
    }

    // build table
    NS_LOG_INFO("build table");
    for (auto& id : m_routerLSA_set) {
        Ptr<OSPFLSA> rtrLSA = m_lsdb.Get(id);
        NS_LOG_INFO("building ... " << *rtrLSA);
        RouterId routerId = rtrLSA->GetHeader()->GetAdvertisingRouter();
        auto rtrBody = rtrLSA->GetBody<OSPFRouterLSABody>();
        for (uint32_t idx = 0, l = rtrBody->CountNeighbors(); idx < l; ++idx) {
            table[routerId][rtrBody->GetNeighborRouterId(idx)] = rtrBody->GetMetric(idx);
            NS_LOG_LOGIC("add route base table: " << routerId << " -> " << rtrBody->GetNeighborRouterId(idx) << ", metric: " << rtrBody->GetMetric(idx));
        }
    }

    NS_LOG_LOGIC("print calculation table for #router: " << m_routerId);
    for (auto& col : table) {
        NS_LOG_LOGIC("[ " << col.first << " ]: ");
        for (auto& row : col.second) {
            NS_LOG_LOGIC("  ( " << row.first << ", " << row.second << " )");
        }
    }

    NS_LOG_INFO("iterate dijkstra's algorithm");
    std::priority_queue<
        std::pair<uint32_t, RouterId>, // cost, id
        std::vector<std::pair<uint32_t, RouterId>>,
        std::greater<std::pair<uint32_t, RouterId>>
    > pq;
    pq.push(std::make_pair(0, m_routerId));
    uint16_t currCost, tmpCost, adjCost;
    RouterId currId, adjRtrId;
    NS_LOG_INFO("iterate dijkstra's algorithm - loop start");
    while (!pq.empty()) {
        currCost = pq.top().first;
        currId = pq.top().second;
        pq.pop();
        NS_LOG_INFO(" currState - id: " << currId << ", cost: " << currCost);
        if (table[currId].size()) {
            for (auto& kv : table[currId]) { // unorderedなので順序保証なし
                adjRtrId = kv.first;
                adjCost = kv.second;
                tmpCost = currCost + adjCost;
                if (
                    tmpCost < costs[adjRtrId] || (
                        tmpCost == costs[adjRtrId] && currId < prevs[adjRtrId] // 等しかったらID若い方を優先
                    )
                ) {
                    costs[adjRtrId] = tmpCost;
                    prevs[adjRtrId] = currId;
                    pq.push(std::make_pair(tmpCost, adjRtrId));
                    NS_LOG_LOGIC(" pushState - id: " << adjRtrId << ", cost: " << tmpCost);
                }
            }
        }
    }

    // ネクストホップ復元 - prevs[i]はi番目に行くためのネクストホップを格納
    std::unordered_set<RouterId> directConnected_set;
    std::vector<RouterId> directConnected_ids;
    NS_LOG_INFO("# rebuild nexthops");
    for (uint32_t i = 0, l = routers; i < l; ++i) {
        if (prevs[i] == m_routerId) {
            directConnected_set.insert(i);
            directConnected_ids.push_back(i);
        }
    }
    for (uint32_t i = 0, l = routers; i < l; ++i) {
        while (prevs[i] && !directConnected_set.count(prevs[i])) {
            prevs[i] = prevs[prevs[i]];
        }
    }
    for (auto item : directConnected_ids) {
        prevs[item] = item;
    }

    // 別名つけておく
    std::vector<RouterId>& nextHops = prevs;

    NS_LOG_INFO("nextHops: #" << nextHops.size());
    for (int i = 0, l = nextHops.size(); i < l; ++i) {
        NS_LOG_INFO("[ " << i << " ]: " << nextHops[i]);
    }

    NS_LOG_INFO("# rebuild network structure");
    // ルータからネットワークを復元する
    m_routingTable.Clear();
    for (auto& id : m_intraAreaPrefixLSA_set) {
        NS_LOG_INFO("iterate...");
        Ptr<OSPFLSA> lsa = m_lsdb.Get(id);
        RouterId routerId = lsa->GetHeader()->GetAdvertisingRouter();
        Ptr<OSPFIntraAreaPrefixLSABody> body = lsa->GetBody<OSPFIntraAreaPrefixLSABody>();
        bool isSelfOriginated = id.IsOriginatedBy(m_routerId, m_rtrIfaceId_set);

        if (body->GetReferenceType() == OSPF_LSA_TYPE_ROUTER) {
            NS_LOG_INFO("prefixes: " << body->CountPrefixes());
            for (uint32_t idx = 0, l = body->CountPrefixes(); idx < l; ++idx) {
                // self originatedな場合、自明にdirectly connected
                Ipv6RoutingTableEntry* rtentry = new Ipv6RoutingTableEntry();
                Ipv6Prefix prefix(body->GetPrefixLength(idx));
                Ipv6Address address = body->GetPrefixAddress(idx);

                int32_t ifaceIdx = (
                    isSelfOriginated ?
                        m_ipv6->GetInterfaceForPrefix(address, prefix) :
                        GetInterfaceForNeighbor(nextHops[routerId])
                );
                if (ifaceIdx < 0) continue;
                NS_LOG_INFO("address: " << address << ", " << prefix << " , ifaceIdx: " << ifaceIdx);
                *rtentry = Ipv6RoutingTableEntry::CreateNetworkRouteTo(
                    address, // dest address
                    prefix, // prefix
                    Ipv6Address::GetZero(), // nextHop address
                    ifaceIdx // output ifaceIdx
                );
                m_routingTable.AddRoute(*rtentry);
            }
        }
    }
}

int32_t Ipv6OspfRouting::GetInterfaceForNeighbor (RouterId routerId) {
    NS_LOG_FUNCTION(m_routerId << "target: " << routerId);
    for (InterfaceData& ifaceData : m_interfaces) {
        if (ifaceData.HasNeighbor(routerId)) {
            NS_LOG_LOGIC("returns: " << ifaceData.GetInterfaceId());
            return ifaceData.GetInterfaceId();
        }
    }
    NS_LOG_LOGIC("returns -1 by default");
    return -1;
}

} /* namespace ospf */
} /* namespace ns3 */