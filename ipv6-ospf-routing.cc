#include <iomanip>
#include <unordered_map>
#include <queue>
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
    NS_LOG_FUNCTION (this);
    m_routerId = ROUTER_ID_SEED++;
}

Ipv6OspfRouting::~Ipv6OspfRouting ()
{
    NS_LOG_FUNCTION (this);
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
    NS_LOG_FUNCTION (this << ipv6);
    NS_ASSERT (m_ipv6 == 0 && ipv6 != 0);
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
    Socket::SocketErrno &sockerr
) {
    NS_LOG_FUNCTION(m_routerId << this << p << header << oif << sockerr);

    Ipv6Address destination = header.GetDestinationAddress ();
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

    if (destination.IsMulticast ())
    {

        NS_LOG_LOGIC ("RouteOutput ()::Multicast destination");
        // nop
    }

    rtentry = Lookup (destination, oif);
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
        NS_LOG_LOGIC("Link Local Multicast packet received");
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

    return false;
}

void Ipv6OspfRouting::NotifyInterfaceUp (uint32_t ifaceIdx) {
    NS_LOG_FUNCTION(this << ifaceIdx << m_ipv6->GetAddress(ifaceIdx, 0).GetAddress());
    if (ifaceIdx == 0) {
        return;
    }

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];

    ifaceData.SetInterfaceId(ifaceIdx);
    m_rtrIfaceId_set.insert(ifaceIdx);

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
    NS_LOG_FUNCTION (this << ifaceIdx);

    NotifyInterfaceEvent(ifaceIdx, InterfaceEvent::IF_DOWN);
}

void Ipv6OspfRouting::NotifyAddAddress (uint32_t ifaceIdx, Ipv6InterfaceAddress address) {
    NS_LOG_FUNCTION (this << ifaceIdx << address);
    NS_LOG_ERROR (this << " - unimplemented");
}

void Ipv6OspfRouting::NotifyRemoveAddress (uint32_t ifaceIdx, Ipv6InterfaceAddress address) {
    NS_LOG_FUNCTION (this << ifaceIdx << address);
    NS_LOG_ERROR (this << " - unimplemented");
}

void Ipv6OspfRouting::NotifyAddRoute (
    Ipv6Address dst,
    Ipv6Prefix mask,
    Ipv6Address nextHop,
    uint32_t ifaceIdx,
    Ipv6Address prefixToUse
) {
    NS_LOG_FUNCTION (this << dst << mask << nextHop);
    NS_LOG_ERROR (this << " - unimplemented");
}

void Ipv6OspfRouting::NotifyRemoveRoute (
    Ipv6Address dst,
    Ipv6Prefix mask,
    Ipv6Address nextHop,
    uint32_t ifaceIdx,
    Ipv6Address prefixToUse
) {
    NS_LOG_FUNCTION (this << dst << mask << nextHop);
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
    NS_LOG_FUNCTION (m_routerId << ifaceIdx << (bool)m_socketToIfaceIdx.count(socket));

    Address pctSrcAddr;
    Ptr<Packet> packet = socket->RecvFrom(pctSrcAddr);

    Ipv6Header ipv6Header;
    packet->RemoveHeader(ipv6Header);

    Inet6SocketAddress inet6SrcAddr = Inet6SocketAddress::ConvertFrom(pctSrcAddr);
    Ipv6Address srcAddr = inet6SrcAddr.GetIpv6();

    OSPFHeader ospfHeader;
    packet->PeekHeader(ospfHeader);

    NS_LOG_LOGIC("handler's received header: ifaceIdx - " << ifaceIdx << ", addr: " << srcAddr);

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

void Ipv6OspfRouting::OriginateLinkLSA(uint32_t ifaceIdx, bool forceRefresh) {
    NS_LOG_FUNCTION (this << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFLinkStateIdentifier id(OSPF_LSA_TYPE_LINK, ifaceData.GetInterfaceId(), m_routerId);
    if (m_lsdb.Has(id) && !forceRefresh) {
        OSPFLSA& lsa = m_lsdb.Get(id);
        lsa.GetHeader()->SetAge(0);
        lsa.GetHeader()->IncrementSequenceNumber();

        AssignFloodingDestination(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
        return;
    }

    OSPFLSA* lsa = new OSPFLSA();
    lsa->Initialize(OSPF_LSA_TYPE_LINK);
    OSPFLSAHeader& hdr = *lsa->GetHeader();
    hdr.SetAge(0);
    hdr.InitializeSequenceNumber();
    hdr.SetId(ifaceData.GetInterfaceId());
    hdr.SetAdvertisingRouter(m_routerId);
    // hdr.SetChecksum(uint16_t);
    // hdr.SetLength(uint16_t);

    OSPFLinkLSABody& body = dynamic_cast<OSPFLinkLSABody&>(*lsa->GetBody());
    body.SetRtrPriority(1);
    body.SetOptions(0x13); // V6, E, R
    body.SetLinkLocalAddress(ifaceData.GetAddress());

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
        }
    }

    lsa->Print(std::cout);
    bool updateFlag = true;
    if (m_lsdb.Has(id)) {
        updateFlag = !(*m_lsdb.Get(id).GetBody() == *lsa->GetBody());
    }
    m_lsdb.Add(*lsa);
    if (updateFlag) {
        if (m_tableUpdateReducible) {
            m_tableUpdateRequired = true;
        } else {
            CalcRoutingTable();
        }
    }
    ifaceData.AddLinkLocalLSA(m_routerId, *lsa);

    AssignFloodingDestination(*lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
}
void Ipv6OspfRouting::OriginateRouterLSA(bool forceRefresh) {
    NS_LOG_FUNCTION (this);
    OSPFLinkStateIdentifier id(OSPF_LSA_TYPE_ROUTER, m_routerId, m_routerId);
    if (m_lsdb.Has(id) && !forceRefresh) {
        OSPFLSA& lsa = m_lsdb.Get(id);
        lsa.GetHeader()->SetAge(0);
        lsa.GetHeader()->IncrementSequenceNumber();

        AssignFloodingDestination(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
        return;
    }

    OSPFLSA* lsa = new OSPFLSA();
    lsa->Initialize(OSPF_LSA_TYPE_ROUTER);
    OSPFLSAHeader& hdr = *lsa->GetHeader();
    hdr.SetAge(0);
    hdr.InitializeSequenceNumber();
    hdr.SetId(m_routerId);
    hdr.SetAdvertisingRouter(m_routerId);
    // hdr.SetChecksum(uint16_t);
    // hdr.SetLength(uint16_t);

    OSPFRouterLSABody& body = dynamic_cast<OSPFRouterLSABody&>(*lsa->GetBody());

    body.SetOptions(0);

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
            for (auto& kv : ifaceData.GetNeighbors()) { // ひとつしかないはず
                NeighborData& neighData = kv.second;
                neighIfaceId = neighData.GetInterfaceId();
                neighRouterId = neighData.GetRouterId();
                body.AddNeighbor(type, metric, ifaceData.GetInterfaceId(), neighIfaceId, neighRouterId);
                break;
            }
        }
    }

    lsa->Print(std::cout);
    bool updateFlag = true;
    if (m_lsdb.Has(id)) {
        updateFlag = !(*m_lsdb.Get(id).GetBody() == *lsa->GetBody());
    }
    m_lsdb.Add(*lsa);
    if (updateFlag) {
        if (m_tableUpdateReducible) {
            m_tableUpdateRequired = true;
        } else {
            CalcRoutingTable();
        }
    }
    m_routerLSA_set.insert(lsa->GetIdentifier());
    AssignFloodingDestination(*lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
}

void Ipv6OspfRouting::OriginateIntraAreaPrefixLSA(bool forceRefresh) {
    NS_LOG_FUNCTION (this);
    // 各エリアごとに実行すべきだが今の所単一エリア
    // DR用の挙動は別に書いてください

    OSPFLinkStateIdentifier id(OSPF_LSA_TYPE_INTRA_AREA_PREFIX, m_routerId, m_routerId);
    if (m_lsdb.Has(id) && !forceRefresh) {
        OSPFLSA& lsa = m_lsdb.Get(id);
        lsa.GetHeader()->SetAge(0);
        lsa.GetHeader()->IncrementSequenceNumber();

        AssignFloodingDestination(lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
        return;
    }

    OSPFLSA* lsa = new OSPFLSA();
    lsa->Initialize(OSPF_LSA_TYPE_INTRA_AREA_PREFIX);
    OSPFLSAHeader& hdr = *lsa->GetHeader();
    hdr.SetAge(0);
    hdr.InitializeSequenceNumber();
    hdr.SetId(m_routerId);
    hdr.SetAdvertisingRouter(m_routerId);
    // hdr.SetChecksum(uint16_t);
    // hdr.SetLength(uint16_t);

    OSPFIntraAreaPrefixLSABody& body = dynamic_cast<OSPFIntraAreaPrefixLSABody&>(*lsa->GetBody());

    body.SetReferenceType(OSPF_LSA_TYPE_ROUTER);
    body.SetReferenceLinkStateId(0); // 0 indicates the LSA is associated with this router
    body.SetReferenceAdvertisedRouter (m_routerId);

    // FIXME: 本来はエリアごと
    for (uint32_t ifaceIdx = 0, l = m_ipv6->GetNInterfaces (); ifaceIdx < l; ifaceIdx++) {
        InterfaceData& ifaceData = m_interfaces[ifaceIdx];
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
                prefixLength = (onlyLocal || perfectPrefix) ? 128 : addr.GetPrefix().GetPrefixLength();
                options = 0x0; // リンクに存在する唯一のルータの場合は0x2(LA-bit)とし、プレフィクス長も128bitにする（ホストアドレス扱い）
                body.AddPrefix(addr.GetAddress(), prefixLength, metric, options);
            }
        }

        // エリアをまたいで設定された仮想リンクが存在するなら、その相手方のアドレスを128ビット、距離0で追加する
        // FIXME: 書く
    }

    lsa->Print(std::cout);
    bool updateFlag = true;
    if (m_lsdb.Has(id)) {
        updateFlag = !(*m_lsdb.Get(id).GetBody() == *lsa->GetBody());
    }
    m_lsdb.Add(*lsa);
    if (updateFlag) {
        if (m_tableUpdateReducible) {
            m_tableUpdateRequired = true;
        } else {
            CalcRoutingTable();
        }
    }
    m_intraAreaPrefixLSA_set.insert(lsa->GetIdentifier());
    AssignFloodingDestination(*lsa, 0/* FIXME: DR, BDRで壊れるはず */, m_routerId);
}

void Ipv6OspfRouting::OriginateRouterSpecificLSAs (uint32_t ifaceIdx, bool forceRefresh) {
    m_tableUpdateRequired = false;
    m_tableUpdateReducible = true;
    OriginateLinkLSA(ifaceIdx, forceRefresh);
    OriginateRouterLSA(forceRefresh);
    OriginateIntraAreaPrefixLSA(forceRefresh);
    if (m_tableUpdateRequired) {
        CalcRoutingTable();
    }
}

uint16_t Ipv6OspfRouting::CalcMetricForInterface (uint32_t ifaceIdx) {
    NS_LOG_FUNCTION(this << ifaceIdx);
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
    NS_LOG_FUNCTION(this << ifaceIdx << srcAddr);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFHello helloPacket;
    packet->RemoveHeader(helloPacket);

    RouterId neighborRouterId = helloPacket.GetRouterId();

    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);
    if (!neighData.IsInitialized()) {
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
    if (helloPacket.GetRouterPriority() != neighData.GetRouterPriority()) {
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

    if (ifaceIdCache != neighData.GetInterfaceId()) {
        // (re)originate router-LSA
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
    NS_LOG_FUNCTION(this << ifaceIdx << packet);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFDatabaseDescription ddPacket;
    packet->RemoveHeader(ddPacket);

    RouterId neighborRouterId = ddPacket.GetRouterId();
    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);

    // lastReceivedDdPacketに保存する。このときLSAHeader部は本来不要
    neighData.SetLastReceivedDD(ddPacket);

    // InterfaceMTUがこのルータの受け取り可能サイズを超えている場合、断片化しているのでreject
    if (ddPacket.GetMtu() > m_ipv6->GetMtu(ifaceIdx)) {
        return;
    }

    switch (neighData.GetState()) {
    case NeighborState::DOWN:
    case NeighborState::ATTEMPT:
    case NeighborState::TWOWAY:
        return;

    case NeighborState::INIT: {
        NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::TWOWAY_RECEIVED);
        if (!neighData.IsState(NeighborState::EXSTART)) {
            return;
        }
        // fall through
    }
    case NeighborState::EXSTART: {
        if (ddPacket.IsNegotiation() && m_routerId < neighborRouterId) {
            neighData.SetAsSlave();
            neighData.SetSequenceNumber(ddPacket.GetSequenceNumber());
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::NEGOT_DONE);
        } else if (
            !(ddPacket.GetInitFlag() || ddPacket.GetMasterFlag()) &&
            ddPacket.GetSequenceNumber () == neighData.GetSequenceNumber() &&
            m_routerId > neighborRouterId
        ) {
            neighData.SetAsMaster();
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::NEGOT_DONE);
        }
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

    /*
    もしlsTypeが未知またはAS-external-LSA(LS type = 5)でかつ相手がstub areaに属するネイバーなら、SeqNumMismatchを発行して処理をやめる。
    */
    // TODO: LS Typeの確認
    OSPFLinkStateIdentifier identifier;
    for (const OSPFLSAHeader& lsaHeader : ddPacket.GetLSAHeaders()) {
        identifier = lsaHeader.CreateIdentifier();
        if (m_lsdb.Has(identifier)) {
            OSPFLSA &storedLsa = m_lsdb.Get(identifier);
            if (lsaHeader.IsMoreRecentThan(*storedLsa.GetHeader())) {
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
            SendDatabaseDescriptionPacket(ifaceIdx, neighborRouterId);
        }
    } else { // Slave
        neighData.SetSequenceNumber(ddPacket.GetSequenceNumber());
        SendDatabaseDescriptionPacket(ifaceIdx, neighborRouterId);
        if (!ddPacket.GetMoreFlag()) {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::EXCHANGE_DONE);
            Simulator::Schedule(ifaceData.GetRouterDeadInterval(), &NeighborData::ClearLastPacket, &neighData);
            OriginateRouterLSA(true);
        }
    }
}

void Ipv6OspfRouting::ReceiveLinkStateRequestPacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
    NS_LOG_FUNCTION(this << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFLinkStateRequest lsrPacket;
    packet->RemoveHeader(lsrPacket);

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
        return;
    }

    std::vector<OSPFLSA> lsas;
    for (const OSPFLinkStateIdentifier& identifier : lsrPacket.GetLinkStateIdentifiers()) {
        if (m_lsdb.Has(identifier)) {
            lsas.push_back(m_lsdb.Get(identifier));
        } else {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::BAD_LS_REQ);
            return;
        }
    }
    SendLinkStateUpdatePacket(ifaceIdx, lsas, neighborRouterId);
}

void Ipv6OspfRouting::ReceiveLinkStateUpdatePacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
    NS_LOG_FUNCTION(this << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFLinkStateUpdate lsuPacket;
    packet->RemoveHeader(lsuPacket);

    RouterId neighborRouterId = lsuPacket.GetRouterId();
    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);

    // DRやBDRである可能性を考慮していません！

    if (neighData.GetState() < NeighborState::EXCHANGE) {
        return;
    }

    std::vector<OSPFLSAHeader> lsasForDelayedAck;

    for (OSPFLSA& received : lsuPacket.GetLSAs()) {
        OSPFLinkStateIdentifier identifier = received.GetIdentifier();
        bool hasInLSDB = m_lsdb.Has(identifier);
        bool isMoreRecent = (
            hasInLSDB &&
            received.GetHeader()->IsMoreRecentThan(*m_lsdb.Get(identifier).GetHeader())
        );
        bool isSameInstance = (
            hasInLSDB &&
            !isMoreRecent &&
            received.GetHeader()->IsSameInstance(*m_lsdb.Get(identifier).GetHeader())
        );
        bool isSelfOriginated = identifier.IsOriginatedBy(m_routerId, m_rtrIfaceId_set);

        // 1,2,3無視
        // 4
        if (
            received.GetHeader()->GetAge() == g_maxAge &&
            m_lsdb.Has(identifier) == 0 &&
            !ifaceData.HasExchangingNeighbor()
        ) {
            // 13.5 direct ack
            SendLinkStateAckPacket(ifaceIdx, *received.GetHeader(), neighborRouterId);
            break; // dispose
        }

        // 5
        if (!hasInLSDB || isMoreRecent) {
            bool isFlooded = false;
            // 5.a
            if (
                hasInLSDB &&
                !isSelfOriginated &&
                !m_lsdb.IsElapsedMinLsArrival(identifier)
            ) {
                break; // dispose
            } else {
                // 5.b
                // 送ってきたネイバーに送り返す
                DirectAssignFloodingDestination(received, ifaceIdx, neighborRouterId);
                isFlooded = true;
            }

            // 5.c
            RemoveFromAllRxmtList(identifier);

            // 5.d
            m_lsdb.Add(received);
            switch (received.GetHeader()->GetType()) {
                case OSPF_LSA_TYPE_LINK: {
                    ifaceData.AddLinkLocalLSA(neighborRouterId, received);
                } break;
                case OSPF_LSA_TYPE_ROUTER: {
                    m_routerLSA_set.insert(identifier);
                } break;
                case OSPF_LSA_TYPE_INTRA_AREA_PREFIX: {
                    m_intraAreaPrefixLSA_set.insert(identifier);
                } break;
            }
            // 13.2を見よ
            CalcRoutingTable ();

            // 5.e
            if (isMoreRecent && !isFlooded) {
                // delayed ack
                lsasForDelayedAck.push_back(*received.GetHeader());
            }

            // 5.f
            if (isSelfOriginated) {
                // 13.4を見よ
                // 受け取ったLSAがより新しい場合、シーケンス番号をそれより進めてインスタンス再生成し、flooding
                if (isMoreRecent) {
                    NS_LOG_WARN("received lsa is self originated and more recent than stored");
                    // TODO: タイプごとにちゃんと生成する
                    OSPFLSA& stored = m_lsdb.Get(identifier);
                    stored.GetHeader()->SetAge(0);
                    stored.GetHeader()->SetSequenceNumber(
                        received.GetHeader()->GetSequenceNumber() + 1
                    );
                    // flooding
                    AssignFloodingDestination(stored, ifaceIdx, neighborRouterId);
                }

                // LSAを発信したくない場合は受け取ったLSAのLS AgeをMaxAgeにして再フラッディングする
                // 1）LSAがsummary-LSAまたはAS-external-LSAであり、ルータは宛先への（広告可能な）ルートを持たない
                // 2）network-LSAだが、ルータはもうDRでない
                // 3）Link State IDがルータ自身のInterface IDの1つだが、広告ルータとこのルータのルータIDが等しくない
                if (false/* FIXME: やって */) {
                    received.GetHeader()->SetAge(g_maxAge);
                    // flooding
                    AssignFloodingDestination(received, ifaceIdx, neighborRouterId);
                }
            }
        }

        // 6
        if (neighData.HasInRequestList(identifier)) {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::BAD_LS_REQ);
            break;
        }

        // 7
        if (isSameInstance) {
            if (neighData.HasInRxmtList(identifier)) {
                neighData.RemoveFromRxmtList(identifier);
                // もしBDRなら処理があるので、実装する場合は 13. を見ること
            } else {
                // direct ack
                SendLinkStateAckPacket(ifaceIdx, *received.GetHeader(), neighborRouterId);
            }
            
            break;
        }

        // 8
        if (!(isMoreRecent || isSameInstance)) {
            // 受け取ったLSAがDB内のものより古い場合
            if (m_lsdb.Get(identifier).IsDeprecatedInstance()) {
                break;
            } else if (GetLastLSUSentTime() + g_minLsArrival <= ns3::Now()) {
                // LSUでDB内のLSAを直接送り返す
                DirectAssignFloodingDestination(m_lsdb.Get(identifier), ifaceIdx, neighborRouterId);
            }
        }
    }
    SendLinkStateAckPacket(ifaceIdx, lsasForDelayedAck, neighborRouterId);
}

void Ipv6OspfRouting::DirectAssignFloodingDestination (OSPFLSA& lsa, uint32_t ifaceIdx, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(this << ifaceIdx << neighborRouterId);
    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    NeighborData& neighData = ifaceData.GetNeighbor(neighborRouterId);
    neighData.AddRxmtList(lsa);
}

void Ipv6OspfRouting::AssignFloodingDestination (OSPFLSA& lsa, uint32_t receivedIfaceIdx, RouterId senderRouterId) {
    NS_LOG_FUNCTION(this << receivedIfaceIdx << senderRouterId);
    OSPFLSAHeader& lsHdr = *lsa.GetHeader();
    OSPFLinkStateIdentifier identifier = lsa.GetIdentifier();
    bool isAlreadyAddedToRxmtList = false;
    uint32_t targetArea = m_interfaces[receivedIfaceIdx].GetAreaId();

    for (InterfaceData& ifaceData : m_interfaces) {
        if (!ifaceData.IsActive()) continue;
        if (
            lsHdr.GetType() != OSPF_LSA_TYPE_AS_EXTERNAL &&
            ifaceData.GetAreaId() != targetArea
        ) {
            continue; // next iface
        }

        // https://tools.ietf.org/html/rfc5340#section-4.5.2
        if (lsHdr.IsAreaScope()) {
            if (ifaceData.GetAreaId() != targetArea) {
                continue; // next iface
            }
        }

        if (lsHdr.IsLinkLocalScope()) {
            if (!ifaceData.IsKnownLinkLocalLSA(identifier)) {
                continue; // next iface
            }
        }

        isAlreadyAddedToRxmtList = false;
        // 1
        for (auto& kv : ifaceData.GetNeighbors()) {
            NeighborData& neigh = kv.second;
            // 1.a
            if (neigh.GetState() < NeighborState::EXCHANGE) continue; // next neighbor
            // 1.b
            if (!neigh.IsState(NeighborState::FULL)) {
                if (neigh.HasInRequestList(identifier)) {
                    OSPFLSAHeader& lsHdrInReqList = neigh.GetFromRequestList(identifier);
                    if (lsHdrInReqList.IsMoreRecentThan(lsHdr)) {
                        continue; // next neighbor
                    }
                    if (lsHdrInReqList.IsSameInstance(lsHdr)) {
                        neigh.RemoveFromRequestList(identifier);
                        continue; // next neighbor
                    }
                    neigh.RemoveFromRequestList(identifier);
                }
            }
            // 1.c
            if (lsHdr.GetAdvertisingRouter() == neigh.GetRouterId()) continue; // next neighbor

            // 1.d
            neigh.AddRxmtList(lsa);
            isAlreadyAddedToRxmtList = true;
        }

        // 2
        if (!isAlreadyAddedToRxmtList) continue; // next iface

        // 3
        if (
            ifaceData.IsIndex(receivedIfaceIdx) &&
            ifaceData.GetDesignatedRouter() == senderRouterId &&
            ifaceData.GetBackupDesignatedRouter() == senderRouterId
        ) {
            continue; // next iface
        }

        // 4
        if (
            ifaceData.IsIndex(receivedIfaceIdx) &&
            ifaceData.IsState(InterfaceState::BACKUP)
        ) {
            continue;
        }

        // 5
        ifaceData.AddRxmtList(lsa);
    }
}

Time& Ipv6OspfRouting::GetLastLSUSentTime () {
    return m_lastLsuSendTime;
}

void Ipv6OspfRouting::ReceiveLinkStateAckPacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
    NS_LOG_FUNCTION(this << ifaceIdx);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];
    if (!ifaceData.IsActive()) return;

    OSPFLinkStateAck lsaPacket;
    packet->RemoveHeader(lsaPacket);

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
        return;
    }
    
    std::vector<OSPFLSA>& rxmtList = neighData.GetRxmtList();
    std::vector<OSPFLSAHeader>& lsaList = lsaPacket.GetLSAHeaders();
    for (
        auto it = rxmtList.begin();
        it != rxmtList.end();
        // nop
    ) {
        int i = 0, l = lsaList.size();
        while (i < l && !lsaList[i].IsSameIdentity(*(it->GetHeader()))) ++i;
        if (i != l) {
            if (lsaList[i].IsSameInstance(*(it->GetHeader()))) {
                it = rxmtList.erase(it);
            } else {
                NS_LOG_WARN("BAD ACK: ");
            }
        } else {
            ++it;
        }
    }
}

void Ipv6OspfRouting::RemoveFromAllRxmtList(OSPFLinkStateIdentifier &identifier) {
    NS_LOG_FUNCTION(this);
    for (InterfaceData& ifaceData : m_interfaces) {
        if (!ifaceData.IsActive()) continue;
        for (auto& kv : ifaceData.GetNeighbors()) {
            kv.second.RemoveFromRxmtList(identifier);
        }
    }
}

bool Ipv6OspfRouting::IsNeighborToBeAdjacent(uint32_t ifaceIdx, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(this << ifaceIdx << neighborRouterId);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];
    InterfaceType ifaceType = ifaceData.GetType();

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
    NS_LOG_FUNCTION(this << ifaceIdx << event);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

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
                return;
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
                return;
            }


            if (ifaceData.IsType(InterfaceType::NBMA)) {
                NS_LOG_ERROR("NBMA connection is not supported");
                // TODO: このインターフェースの設定済みネイバーリストを検査(NBMAが未実装なのでこの状態にする)
                // 各ネイバーにStartイベントを発行
                return;
            }
        }
        return;
    }
    case InterfaceEvent::BACKUP_SEEN: // fall through
    case InterfaceEvent::WAIT_TIMER: {
        if (ifaceData.IsState(InterfaceState::WAITING)) {
            ifaceData.CalcDesignatedRouter();
        }
        return;
    }
    case InterfaceEvent::NEIGH_CHANGE: {
        if (
            ifaceData.IsState(InterfaceState::DR_OTHER) ||
            ifaceData.IsState(InterfaceState::DR) ||
            ifaceData.IsState(InterfaceState::BACKUP)
        ) {
            ifaceData.CalcDesignatedRouter();
        }
        return;
    }
    case InterfaceEvent::IF_DOWN: {
        ifaceData.SetState(InterfaceState::DOWN);
        OriginateRouterSpecificLSAs(ifaceIdx);
        ifaceData.ResetInstance();
        for (auto& kv : ifaceData.GetNeighbors()) {
            NotifyNeighborEvent(ifaceIdx, kv.first, NeighborEvent::KILL_NBR);
        }
        return;
    }
    case InterfaceEvent::LOOP_IND: {
        ifaceData.SetState(InterfaceState::LOOPBACK);
        OriginateRouterSpecificLSAs(ifaceIdx);
        ifaceData.ResetInstance();
        for (auto& kv : ifaceData.GetNeighbors()) {
            NotifyNeighborEvent(ifaceIdx, kv.first, NeighborEvent::KILL_NBR);
        }
        return;
    }
    case InterfaceEvent::UNLOOP_IND: {
        if (ifaceData.IsState(InterfaceState::LOOPBACK)) {
            ifaceData.SetState(InterfaceState::DOWN);
            OriginateRouterSpecificLSAs(ifaceIdx);
        }
        return;
    }
    } // switch
}

void Ipv6OspfRouting::NotifyNeighborEvent(uint32_t ifaceIdx, RouterId neighborRouterId, NeighborEvent event) {
    NS_LOG_FUNCTION(this << ifaceIdx << neighborRouterId << event);
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];
    NeighborData &neighbor = ifaceData.GetNeighbor(neighborRouterId);

    // OSPFv2 10.3 The Neighbor state machine
    // https://tools.ietf.org/html/rfc2328#page-89
    switch (event) {
    case NeighborEvent::START: {
        if (neighbor.IsState(NeighborState::DOWN)) {
            neighbor.SetState(NeighborState::ATTEMPT);
            // Hello送信はInterfaceUpで起動している
            neighbor.GetInactivityTimer().Schedule();
        }
        return;
    }
    case NeighborEvent::HELLO_RECEIVED: {
        if (neighbor.IsState(NeighborState::DOWN) ||
            neighbor.IsState(NeighborState::ATTEMPT))
        {
            neighbor.SetState(NeighborState::INIT);
        }
        neighbor.GetInactivityTimer().Schedule();
        return;
    }
    case NeighborEvent::TWOWAY_RECEIVED: {
        if (neighbor.IsState(NeighborState::INIT)) {
            if (IsNeighborToBeAdjacent(ifaceIdx, neighborRouterId)) {
                neighbor.SetState(NeighborState::EXSTART);
                neighbor.StartExchange();
                SendDatabaseDescriptionPacket(ifaceIdx, neighborRouterId, true);
            } else {
                neighbor.SetState(NeighborState::TWOWAY);
            }
        }
        return;
    }
    case NeighborEvent::NEGOT_DONE: {
        if (neighbor.IsState(NeighborState::EXSTART)) {
            neighbor.SetState(NeighborState::EXCHANGE);

            // FIXME: tooooo ad-hoc
            std::vector<OSPFLSAHeader> summary, summary2;
            std::vector<OSPFLSA> rxmt;
            m_lsdb.GetSummary(summary, rxmt);

            for(auto& hdr : summary) {
                if(ifaceData.IsKnownLinkLocalLSA(hdr.CreateIdentifier())) {
                    summary2.push_back(hdr);
                }
            }

            neighbor.SetSummaryList(summary2);
            for(auto& maxAgedLsa : rxmt) {
                neighbor.AddRxmtList(maxAgedLsa);
            }
            SendDatabaseDescriptionPacket(ifaceIdx, neighborRouterId);
        }
        return;
    }
    case NeighborEvent::EXCHANGE_DONE: {
        if (neighbor.IsState(NeighborState::EXCHANGE)) {
            if (neighbor.IsRequestListEmpty()) {
                neighbor.SetState(NeighborState::FULL);
                OriginateRouterSpecificLSAs(ifaceIdx);
            } else {
                neighbor.SetState(NeighborState::LOADING);
                SendLinkStateRequestPacket(ifaceIdx, neighborRouterId);
            }
        }
        return;
    }
    case NeighborEvent::LOADING_DONE: {
        if (neighbor.IsState(NeighborState::LOADING)) {
            neighbor.SetState(NeighborState::FULL);
            OriginateRouterSpecificLSAs(ifaceIdx);
        }
        return;
    }
    case NeighborEvent::IS_ADJ_OK: {
        if (neighbor.IsState(NeighborState::TWOWAY)) {
            if (IsNeighborToBeAdjacent(ifaceIdx, neighborRouterId)) {
                neighbor.SetState(NeighborState::EXSTART);
                neighbor.StartExchange();
                SendDatabaseDescriptionPacket(ifaceIdx, neighborRouterId, true);
            }
            return;
        }
        if (neighbor.GetState() >= NeighborState::EXSTART) {
            if (!IsNeighborToBeAdjacent(ifaceIdx, neighborRouterId)) {
                neighbor.SetState(NeighborState::TWOWAY);
                neighbor.ClearList();
            }
        }
        return;
    }
    case NeighborEvent::BAD_LS_REQ: // fall through
    case NeighborEvent::SEQ_NUM_MISMATCH: {
        if (neighbor.GetState() >= NeighborState::EXCHANGE) {
            neighbor.SetState(NeighborState::EXSTART);
            neighbor.ClearList();
            neighbor.StartExchange();
            SendDatabaseDescriptionPacket(ifaceIdx, neighborRouterId, true);
        }
        return;
    }
    case NeighborEvent::KILL_NBR: // fall through
    case NeighborEvent::LL_DOWN: // fall through
    case NeighborEvent::INACTIVE: {
        neighbor.SetState(NeighborState::DOWN);
        neighbor.ClearList();
        return;
    }
    case NeighborEvent::ONEWAY_RECEIVED: {
        if (neighbor.GetState() >= NeighborState::TWOWAY) {
            neighbor.SetState(NeighborState::INIT);
            neighbor.ClearList();
        }
        return;
    }
    } // switch
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
    NS_LOG_LOGIC("SendTo result: " << res);
    
    Simulator::ScheduleNow(&InterfaceData::ScheduleHello, &ifaceData);
}

void Ipv6OspfRouting::SendDatabaseDescriptionPacket(uint32_t ifaceIdx, RouterId neighborRouterId, bool isInit) {
    NS_LOG_FUNCTION(this << ifaceIdx << neighborRouterId);

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
    dd.SetSequenceNumber(neighData.GetSequenceNumber());
    if (!isInit) {
        dd.SetLSAHeaders(neighData.GetSummary(mtu - 40));
    }

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(dd);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    socket->SendTo(packet, 0, Inet6SocketAddress(neighData.GetAddress(), PROTO_PORT));
}
void Ipv6OspfRouting::SendLinkStateRequestPacket(uint32_t ifaceIdx, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(this << ifaceIdx << neighborRouterId);

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    NeighborData& neighData = ifaceData.GetNeighbor(neighborRouterId);

    OSPFLinkStateRequest lsr;

    lsr.SetRouterId(m_routerId);
    lsr.SetAreaId(ifaceData.GetAreaId());
    lsr.SetInstanceId(0);
    // lsr.SetOptions(0x13); // V6, E, R
    uint32_t mtu = m_ipv6->GetMtu(ifaceIdx);
    std::vector<OSPFLinkStateIdentifier> lsids;
    for (OSPFLSAHeader& lsHdr : neighData.GetRequestList(mtu - 20)) {
        lsids.push_back(lsHdr.CreateIdentifier());
    }
    lsr.SetLinkStateIdentifiers(lsids);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(lsr);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    socket->SendTo(packet, 0, Inet6SocketAddress(neighData.GetAddress(), PROTO_PORT));
}
void Ipv6OspfRouting::SendLinkStateAckPacket(uint32_t ifaceIdx, OSPFLSAHeader& lsaHeader, RouterId neighborRouterId) {
    std::vector<OSPFLSAHeader> lsaHdrs;
    lsaHdrs.push_back(lsaHeader);
    Ipv6OspfRouting::SendLinkStateAckPacket(ifaceIdx, lsaHdrs, neighborRouterId);
}
void Ipv6OspfRouting::SendLinkStateUpdatePacket() {
    for (int i = 0, l = m_interfaces.size(); i < l; ++i) {
        if (m_interfaces[i].IsActive()) {
            SendLinkStateUpdatePacket(i);
        }
    }
}
void Ipv6OspfRouting::SendLinkStateUpdatePacket(uint32_t ifaceIdx) {
    for (auto& kv : m_interfaces[ifaceIdx].GetNeighbors()) {
        SendLinkStateUpdatePacket(ifaceIdx, kv.first);
    }
}
void Ipv6OspfRouting::SendLinkStateUpdatePacket(uint32_t ifaceIdx, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(this << ifaceIdx << neighborRouterId);

    if (neighborRouterId == 0) return;

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    NeighborData& neighData = ifaceData.GetNeighbor(neighborRouterId);
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
    lsu.SetLSAs(neighData.GetRxmtList(mtu - 40));

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(lsu);

    Ptr<Socket> socket = m_ifaceIdxToSocket[ifaceIdx];
    socket->SendTo(packet, 0, Inet6SocketAddress(dstAddr, PROTO_PORT));
}
void Ipv6OspfRouting::SendLinkStateUpdatePacket(uint32_t ifaceIdx, std::vector<OSPFLSA>& lsas, RouterId neighborRouterId) {
    NS_LOG_FUNCTION(this << ifaceIdx << neighborRouterId);
    if (neighborRouterId == 0) return;

    InterfaceData& ifaceData = m_interfaces[ifaceIdx];
    Ipv6Address dstAddr = (
        ifaceData.GetType() == InterfaceType::P2P ?
            Ipv6OspfRouting::AllSPFRouters :
            ifaceData.GetNeighbor(neighborRouterId).GetAddress()
    );

    OSPFLinkStateUpdate lsu;
    lsu.SetRouterId(m_routerId);
    lsu.SetAreaId(ifaceData.GetAreaId());
    lsu.SetInstanceId(0);

    std::vector<OSPFLSA> partialLsa;
    uint32_t mtu = m_ipv6->GetMtu(ifaceIdx);
    uint32_t maxBytes = mtu - 40;
    while (!lsas.empty()) {
        maxBytes -= lsas.back().GetSerializedSize();
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
        SendLinkStateUpdatePacket(ifaceIdx, lsas, neighborRouterId);
    }
}
void Ipv6OspfRouting::SendLinkStateAckPacket(uint32_t ifaceIdx, std::vector<OSPFLSAHeader>& lsaHeaders, RouterId neighborRouterId) {
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

Ptr<Ipv6Route> Ipv6OspfRouting::Lookup(Ipv6Address dst, Ptr<NetDevice> interface) {
    NS_LOG_FUNCTION(this << dst << interface);

    return 0;
}

void Ipv6OspfRouting::CalcRoutingTable (bool recalcAll) {
    NS_LOG_FUNCTION(this << recalcAll);
    uint32_t routers = ROUTER_ID_SEED;

    std::vector<std::unordered_map<RouterId, uint16_t> > table(routers, std::unordered_map<RouterId, uint16_t>());
    uint16_t INFCOST = 65535;
    std::vector<uint16_t> costs(routers, 65535); // キーの存在確認をすること、デフォルトは無限大
    std::vector<RouterId> prevs(routers, 0); // 0は経路なしなので存在確認不要
    costs[m_routerId] = 0;

    // build table
    for (auto& id : m_routerLSA_set) {
        OSPFLSA& rtrLSA = m_lsdb.Get(id);
        RouterId routerId = rtrLSA.GetHeader()->GetAdvertisingRouter();
        Ptr<OSPFRouterLSABody> rtrBody = rtrLSA.GetBody<OSPFRouterLSABody>();
        for (uint32_t idx = 0, l = rtrBody->CountNeighbors(); idx < l; ++idx) {
            table[routerId][rtrBody->GetNeighborRouterId(idx)] = rtrBody->GetMetric(idx);
            std::cout << "add route base table: " << routerId << " -> " << rtrBody->GetNeighborRouterId(idx) << "\n";
        }
    }

    // iterate dijkstra's algorithm
    std::priority_queue<
        std::pair<uint32_t, RouterId>, // cost, id
        std::vector<std::pair<uint32_t, RouterId>>,
        std::greater<std::pair<uint32_t, RouterId>>
    > pq;
    pq.push(std::make_pair(0, m_routerId));
    uint16_t currCost, tmpCost;
    RouterId currId;
    std::cout << "Calc Route\n";
    while (!pq.empty()) {
        currCost = pq.top().first;
        currId = pq.top().second;
        pq.pop();
        std::cout << " currState - id: " << currId << ", cost: " << currCost << "\n";
        for(auto& kv : table[currId]) {
            tmpCost = currCost + kv.second;
            std::cout << " connection - " << currId << " -> " << kv.first << ", cost: " << kv.second << ", sum: " << tmpCost << "\n";
            if (tmpCost < costs[kv.first]) {
                costs[kv.first] = tmpCost;
                prevs[kv.first] = currId;
                pq.push(std::make_pair(tmpCost, kv.first));
                std::cout << " pushState - id: " << kv.first << ", cost: " << tmpCost << "\n";
            }
        }
    }

    std::cout << "Print Route\n";
    for (int i = 0, l = routers; i < l; ++i) {
        std::cout << "  prev " << i << " : " << prevs[i] << "\n";
        std::cout << "  cost " << i << " : " << costs[i] << "\n";
    }

    // ルータからネットワークを復元する
    m_routingTable.Clear();
    for (auto& id : m_intraAreaPrefixLSA_set) {
        OSPFLSA& lsa = m_lsdb.Get(id);
        RouterId routerId = lsa.GetHeader()->GetAdvertisingRouter();
        Ptr<OSPFIntraAreaPrefixLSABody> body = lsa.GetBody<OSPFIntraAreaPrefixLSABody>();
        for (uint32_t idx = 0, l = body->CountPrefixes(); idx < l; ++idx) {
            // table[routerId][body->GetNeighborRouterId(idx)] = body->GetMetric(idx);
        }
    }
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