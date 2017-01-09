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

#include "ospf-hello.h"
#include "ospf-database-description.h"
#include "ospf-link-state-request.h"
#include "ospf-link-state-update.h"
#include "ospf-link-state-ack.h"

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
    m_routerId = ROUTER_ID_SEED++;
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
    InterfaceData& ifaceData = m_interfaces[interface];

    ifaceData.SetInterfaceId(interface);

    Timer& waitTimer = ifaceData.GetWaitTimer();
    waitTimer.SetFunction(&Ipv6OspfRouting::NotifyInterfaceEvent, this);
    waitTimer.SetArguments(interface, InterfaceEvent::WAIT_TIMER);

    ifaceData.SetHelloSender(MakeCallback(&Ipv6OspfRouting::SendHelloPacket, this));
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

void NotifyInterfaceEvent(uint32_t ifaceIdx, InterfaceEvent event) {
    // m_interfaces[ifaceIdx].NotifyEvent(event);
}

void Ipv6OspfRouting::HandleProtocolMessage(Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);

    // 1. checksumが正しいか(ignore)
    // 2. dstAddrが受け取ったIfaceのアドレス、またはマルチキャストのAllSPFRouters, AllDRouters、あるいはグローバルアドレスか
    // 3. IP Protocolが89になっているか
    // 4. 自分が送信したパケットでないか

    // - AreaIdが正しいか(ignore)
    // - AllDRouters宛のとき、受け取ったIfaceがDRまたはBDRか
    // - AuTypeはそのエリアで有効なものか(ignore)
    // - パケットが認証されているか(ignore)

    // 各パケットごとの受け取り実装に移譲する

    uint32_t ifaceIdx = m_socketToIfaceIdx[socket];

    Address pctSrcAddr;
    Ptr<Packet> packet = socket->RecvFrom(pctSrcAddr);

    Inet6SocketAddress inet6SrcAddr = Inet6SocketAddress::ConvertFrom(pctSrcAddr);
    Ipv6Address srcAddr = inet6SrcAddr.GetIpv6();

    OSPFHeader ospfHeader;
    packet->PeekHeader(ospfHeader);

    uint8_t ospfPacketType = ospfHeader.GetType();
    switch (ospfPacketType) {
        case OSPF_TYPE_HELLO: 
            return ReceiveHelloPacket(ifaceIdx, srcAddr, packet);
        case OSPF_TYPE_DATABASE_DESCRIPTION: 
            return ReceiveDatabaseDescriptionPacket(ifaceIdx, packet);
        case OSPF_TYPE_LINK_STATE_REQUEST:
            return ReceiveLinkStateRequestPacket(ifaceIdx, packet);
        case OSPF_TYPE_LINK_STATE_UPDATE:
            return ReceiveLinkStateUpdatePacket(ifaceIdx, packet);
        case OSPF_TYPE_LINK_STATE_ACK:
            return ReceiveLinkStateAckPacket(ifaceIdx, packet);
        default:
            NS_LOG_WARN("unknown ospf packet type: " << ospfPacketType);
    }

}

void Ipv6OspfRouting::ReceiveHelloPacket(uint32_t ifaceIdx, Ipv6Address srcAddr, Ptr<Packet> packet) {
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFHello helloPacket;
    packet->RemoveHeader(helloPacket);

    RouterId neighborRouterId = helloPacket.GetRouterId();

    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);
    if (!neighData.IsInitialized()) {
        neighData.MinimalInitialize(srcAddr, helloPacket);
    }

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

    neighData.Initialize(srcAddr, helloPacket);

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
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

    OSPFDatabaseDescription ddPacket;
    packet->RemoveHeader(ddPacket);

    RouterId neighborRouterId = ddPacket.GetRouterId();
    NeighborData &neighData = ifaceData.GetNeighbor(neighborRouterId);

    // lastReceivedDdPacketに保存する。このときLSAHeader部は不要
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
                ddPacket.GetSequenceNumber() == neighData.GetSequenceNumber()
            ) || (
                neighData.IsSlave() &&
                ddPacket.GetSequenceNumber() > neighData.GetSequenceNumber()
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
    default: {}
    } // switch

    // accepted

    /*
    もしlsTypeが未知またはAS-external-LSA(LS type = 5)でかつ相手がstub areaに属するネイバーなら、SeqNumMismatchを発行して処理をやめる。
    */
    // TODO: LS Typeの確認
    OSPFLinkStateIdentifier identifier;
    for (const OSPFLSAHeader& lsaHeader : ddPacket.GetLSAHeaders()) {
        identifier = lsaHeader.CreateIdentifier();
        if (m_lsdb.count(identifier)) {
            OSPFLSA &storedLsa = m_lsdb[identifier];
            if (lsaHeader.IsNewerThan(*storedLsa.GetHeader())) {
                neighData.AddLinkStateRequestList(lsaHeader);
            }
        } else {
            neighData.AddLinkStateRequestList(lsaHeader);
        }
    }

    if (neighData.IsMaster()) {
        neighData.IncrementSequenceNumber();
        if (neighData.IsExchangeDone() && !ddPacket.GetMoreFlag()) {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::EXCHANGE_DONE);
        } else {
            // SendDatabaseDescriptionPacket
        }
    } else { // Slave
        neighData.SetSequenceNumber(ddPacket.GetSequenceNumber());
        // SendDatabaseDescriptionPacket
        NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::EXCHANGE_DONE);
    }
    
}

void Ipv6OspfRouting::ReceiveLinkStateRequestPacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
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
        neighData.GetState(NeighborState::EXCHANGE) ||
        neighData.GetState(NeighborState::LOADING) ||
        neighData.GetState(NeighborState::FULL)
    )) {
        return;
    }

    std::vector<OSPFLSA> lsas;
    for (const OSPFLinkStateIdentifier& identifier : lsrPacket.GetLinkStateIdentifiers()) {
        if (m_lsdb.count(identifier)) {
            lsas.push_back(m_lsdb[identifier]);
        } else {
            NotifyNeighborEvent(ifaceIdx, neighborRouterId, NeighborEvent::BAD_LS_REQ);
            return;
        }
    }
    SendLinkStateUpdatePacket(ifaceIdx, lsas, neighborRouterId);
}

void ReceiveLinkStateUpdatePacket(uint32_t ifaceIdx, Ipv6Address srcAddr, Ptr<Packet> packet) {
    /*
    Exchange以下の場合は無視する
1. LSAのLSチェックサムを確認します。不正であれば破棄して次のLSAを見ます。
2. LSAのLSタイプを確認します。unknownであれば破棄して次のLSAを見ます。
3. あるいは、LSタイプが5(AS-external-LSA)であれば……（今回は無視）
4. あるいは、LSAのageがMaxAgeと等しく、かつルータのLSDBにそのLSAが存在せず、さらに近隣ルータにExchangeかLoading状態なものが存在しない場合、次の手順をとります。
    1. LSAckを近隣ルータに送信します。
    2. LSAを破棄して次のLSAを見ます。
5. LSDBにLSAのインスタンスがあるか探します。存在しないか、今受け取ったLSAの方がより新しい場合（新しさの確認方法はSection13.1で）は、次の手順をとります。
    1. すでにコピーが存在して、フラッディングにより受け取ったものであって、それが追加されてからMinLSArrival経っていない場合、破棄して次のLSAを見ます。
    2. そうでなければこのLSAをすぐにインターフェースからフラッディングします。
    3. 全てのリンク状態再送信リストから現在のDBのコピーを削除します。
    4. 新しいLSAをLSDBにインストールします。ルーティングテーブルの再計算がスケジューリングされるかもしれません。新たなLSAの追加時刻はこの段階での時刻を使います。フラッディング機構はここでインストールされたLSAをMinLSArrival秒が経過するまで上書きできません。LSAのインストールについてはSection 13.2を参照してください。
    5. 場合によってはLSAを受け取った証としてLSAckを、受信したインターフェースから送り返すかもしれません。Section13.5を参照してください。
        1. 1で既存のものより新しいLSAを受け取っており、2でフラッディングが行われた場合は、LSAckを受信したインターフェースから送り返す。
        2. 2でフラッディングが行われた場合はLSAckは送らない。
    6. もしこの新たなLSAが自分自身が発行したものであった場合、なんかよくわかんないから実装しない
6. 送信中近隣リンク状態リクエストリストの中にLSAを見つけた場合、データベース交換のプロセスにエラーが生じているということです。この場合は、近隣イベントのBadLSReqを送信してLSUの処理を中断させることにより、データベース交換プロセスを再始動します。
7. 全く同じインスタンスをLSDBの中に見つけた場合、次の2つの手順で処理されます。
    1. LSAがリンク状態再送信リストの中にある場合、このルータ自身がこのLSAに対する確認応答を待っているところです。ルータは受け取ったLSAを確認応答として扱い、リンク状態再送信リストから削除します。これは「暗黙の確認応答」と呼ばれるものです。確認応答プロセスSection13.5を参照してください。
    2. 場合によってはLSAを受け取った証としてLSAckを、受信したインターフェースから送り返すかもしれません。Section13.5を参照してください。
        1. 暗黙の確認応答として扱った場合、BDRはなんかいろいろやって送り返し、そうでなければ送らない。
        2. 暗黙の確認応答として扱わない場合、直接確認応答を送り返す。
8. LSDB内のデータの方が新しい場合、DB内のもののLS ageがMaxAgeと等しくかつLSシーケンス番号がMaxSequenceNumberに等しい場合は、単に応答なく破棄します。そうでない場合は、直近のMinLSArrival秒からまだ送信されていないぶんを、LSUとして当該近隣ルータに送信します。このときLSUは近隣ルータに直接送信する必要があり、これをリンク状態再送信リストには入れず、確認応答は送信しないでください。
    */
}

void Ipv6OspfRouting::ReceiveLinkStateAckPacket(uint32_t ifaceIdx, Ptr<Packet> packet) {
    InterfaceData &ifaceData = m_interfaces[ifaceIdx];

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
    
    std::vector<OSPFLSAHeader>& rxmtList = neighData.GetRxmtList();
    std::vector<OSPFLSAHeader>& lsaList = lsaPacket.GetLSAHeaders();
    for (
        auto it = rxmtList.begin();
        it != rxmtList.end();
        // nop
    ) {
        int i = 0;
        for (int l = lsaList.size(); i < l; ++i) {
            if (lsaList[i].IsSameIdentity(*it)) {
                break;
            }
        }
        if (i != l) {
            if (lsaList[i].IsSameInstance(*it)) {
                it = rxmtList.erase(it)
            } else {
                NS_LOG_WARN("BAD ACK");
            }
        } else {
            ++it;
        }
    }
}

bool Ipv6OspfRouting::IsNeighborToBeAdjacent(uint32_t ifaceIdx, RouterId neighborRouterId) {
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
    InterfaceData &interface = m_interfaces[ifaceIdx];

    // OSPFv2 10.3 The Interface state machine
    // https://tools.ietf.org/html/rfc2328#page-72
    switch (event) {
    case InterfaceEvent::IF_UP: {
        if (interface.IsState(InterfaceState::DOWN)) {
            // TODO: Helloのタイマーを起動

            if (
                interface.IsType(InterfaceType::P2P) ||    
                interface.IsType(InterfaceType::P2M) ||    
                interface.IsType(InterfaceType::VIRTUAL)    
            ) {
                interface.SetState(InterfaceState::P2P);
                return;
            }

            if (interface.IsEligibleToDR()) {
                if (
                    interface.IsType(InterfaceType::BROADCAST) ||
                    interface.IsType(InterfaceType::NBMA)
                ) {
                    interface.SetState(InterfaceState::WAITING);
                    interface.StartWaitTimer();
                }
            } else {
                interface.SetState(InterfaceState::DR_OTHER);
                return;
            }


            if (interface.IsType(InterfaceType::NBMA)) {
                NS_LOG_ERROR("NBMA connection is not supported");
                // このインターフェースの設定済みネイバーリストを検査(NBMAが未実装なのでこの状態にする)
                // 各ネイバーにStartイベントを発行
                return;
            }
        }
        return;
    }
    case InterfaceEvent::BACKUP_SEEN: // fall through
    case InterfaceEvent::WAIT_TIMER: {
        if (interface.IsState(InterfaceState::WAITING)) {
            interface.CalcDesignatedRouter();
        }
        return;
    }
    case InterfaceEvent::NEIGH_CHANGE: {
        if (
            interface.IsState(InterfaceState::DR_OTHER) ||
            interface.IsState(InterfaceState::DR) ||
            interface.IsState(InterfaceState::BACKUP)
        ) {
            interface.CalcDesignatedRouter();
        }
        return;
    }
    case InterfaceEvent::IF_DOWN: {
        interface.SetState(InterfaceState::DOWN);
        interface.ResetInstance();
        for (auto& kv : interface.GetNeighbors()) {
            NotifyNeighborEvent(ifaceIdx, kv.first, NeighborEvent::KILL_NBR);
        }
        return;
    }
    case InterfaceEvent::LOOP_IND: {
        interface.SetState(InterfaceState::LOOPBACK);
        interface.ResetInstance();
        for (auto& kv : interface.GetNeighbors()) {
            NotifyNeighborEvent(ifaceIdx, kv.first, NeighborEvent::KILL_NBR);
        }
        return;
    }
    case InterfaceEvent::UNLOOP_IND: {
        if (interface.IsState(InterfaceState::LOOPBACK)) {
            interface.SetState(InterfaceState::DOWN);
        }
        return;
    }
    } // switch
}

void Ipv6OspfRouting::NotifyNeighborEvent(uint32_t ifaceIdx, RouterId neighborRouterId, NeighborEvent event) {
    InterfaceData &interface = m_interfaces[ifaceIdx];
    NeighborData &neighbor = interface.GetNeighbor(neighborRouterId);

    // OSPFv2 10.3 The Neighbor state machine
    // https://tools.ietf.org/html/rfc2328#page-89
    switch (event) {
    case NeighborEvent::START: {
        if (neighbor.IsState(NeighborState::DOWN)) {
            neighbor.SetState(NeighborState::ATTEMPT);
            // SendHello();
            // neighbor.SetInactivityTimerCallback(&InterfaceData::NotifyNeighborEvent, this);
            // neighbor.SetInactivityTimerArguments(neighborRouterId, NeighborEvent::INACTIVE);
            // neighbor.ResetInactivityTimer(m_routerDeadInterval);
        }
        return;
    }
    case NeighborEvent::HELLO_RECEIVED: {
        if (neighbor.IsState(NeighborState::DOWN) ||
            neighbor.IsState(NeighborState::ATTEMPT))
        {
            neighbor.SetState(NeighborState::INIT);
        }
        // neighbor.SetInactivityTimerCallback(&InterfaceData::NotifyNeighborEvent, this);
        // neighbor.SetInactivityTimerArguments(neighborRouterId, NeighborEvent::INACTIVE);
        // neighbor.ResetInactivityTimer(m_routerDeadInterval);
        return;
    }
    case NeighborEvent::TWOWAY_RECEIVED: {
        if (neighbor.IsState(NeighborState::INIT)) {
            if (IsNeighborToBeAdjacent(ifaceIdx, neighborRouterId)) {
                neighbor.SetState(NeighborState::EXSTART);
                neighbor.StartExchange();
                // SendDatabaseDescription(neighborRouterId);
            } else {
                neighbor.SetState(NeighborState::TWOWAY);
            }
        }
        return;
    }
    case NeighborEvent::NEGOT_DONE: {
        if (neighbor.IsState(NeighborState::EXSTART)) {
            neighbor.SetState(NeighborState::EXCHANGE);
            // DDによるやりとりがはじまる (OSPFv2の10.8と10.6を参照)
            // lsdbSummaryListに突っ込んだりする
            // SendDatabaseDescription(neighborRouterId);
        }
        return;
    }
    case NeighborEvent::EXCHANGE_DONE: {
        if (neighbor.IsState(NeighborState::EXCHANGE)) {
            if (neighbor.IsRequestListEmpty()) {
                neighbor.SetState(NeighborState::FULL);
            } else {
                neighbor.SetState(NeighborState::LOADING);
                // SendLinkStateRequest(neighborRouterId);
            }
        }
        return;
    }
    case NeighborEvent::LOADING_DONE: {
        if (neighbor.IsState(NeighborState::LOADING)) {
            neighbor.SetState(NeighborState::FULL);
        }
        return;
    }
    case NeighborEvent::IS_ADJ_OK: {
        if (neighbor.IsState(NeighborState::TWOWAY)) {
            if (IsNeighborToBeAdjacent(ifaceIdx, neighborRouterId)) {
                neighbor.SetState(NeighborState::EXSTART);
                neighbor.StartExchange();
                // SendDatabaseDescription(neighborRouterId);
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
            // SendDatabaseDescription(neighborRouterId);
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

void Ipv6OspfRouting::SendHelloPacket(uint32_t ifaceIdx, RouterId neighborRouterId) {}
void Ipv6OspfRouting::SendDatabaseDescriptionPacket(uint32_t ifaceIdx, RouterId neighborRouterId) {}
void Ipv6OspfRouting::SendLinkStateRequestPacket(uint32_t ifaceIdx, RouterId neighborRouterId) {}
void Ipv6OspfRouting::SendLinkStateUpdatePacket(uint32_t ifaceIdx, std::vector<OSPFLSA> lsas, RouterId neighborRouterId) {}
void Ipv6OspfRouting::SendLinkStateAckPacket(uint32_t ifaceIdx, RouterId neighborRouterId) {}

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