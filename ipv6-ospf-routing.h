#ifndef IPV6_OSPFV3_ROUTING_H
#define IPV6_OSPFV3_ROUTING_H

#include <stdint.h>

#include <list>

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ospf-routing-table.h"
#include "ospf-struct-interface.h"
#include "ospf-link-state-database.h"
#include "ospf-lsa-identifier.h"

namespace ns3 {
namespace ospf {

// class Packet;
// class NetDevice;
// class Ipv6Interface;
// class Ipv6Route;
// class Node;
// class Ipv6RoutingTableEntry;
// class Ipv6MulticastRoutingTableEntry;
typedef uint32_t RouterId;

class Ipv6OspfRouting : public Ipv6RoutingProtocol {

private:
    static const uint32_t PROTO_PORT = 7345;
    static uint32_t ROUTER_ID_SEED;

    static const Ipv6Address AllSPFRouters;
    static const Ipv6Address AllDRRouters;

    uint32_t m_routerId;
    uint32_t m_knownMaxRouterId;

    typedef std::map< Ptr<Socket>, uint32_t > SocketToIfaceIdx;
    typedef std::map< uint32_t, Ptr<Socket> > IfaceIdxToSocket;

    SocketToIfaceIdx m_socketToIfaceIdx;
    IfaceIdxToSocket m_ifaceIdxToSocket;
    SocketToIfaceIdx m_llmSocketToIfaceIdx;
    IfaceIdxToSocket m_ifaceIdxToLlmSocket;
    std::vector<InterfaceData> m_interfaces;
    RoutingTable m_routingTable;
    OSPFLSDB m_lsdb;
    Time m_lastLsuSendTime;
    std::set<uint32_t> m_rtrIfaceId_set;

    // area data structureだが簡単のために書いてしまう
    std::set<OSPFLinkStateIdentifier> m_intraAreaPrefixLSA_set;
    std::set<OSPFLinkStateIdentifier> m_routerLSA_set;

    bool m_tableUpdateRequired = false;
    bool m_tableUpdateReducible = false;

    /**
    * \brief Ipv6 reference.
    */
    Ptr<Ipv6> m_ipv6;

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
    virtual void ReceiveDatabaseDescriptionPacket(uint32_t ifaceIdx, Ptr<Packet> packet);
    virtual void ReceiveLinkStateRequestPacket(uint32_t ifaceIdx, Ptr<Packet> packet);
    virtual void ReceiveLinkStateUpdatePacket(uint32_t ifaceIdx, Ptr<Packet> packet);
    virtual void ReceiveLinkStateAckPacket(uint32_t ifaceIdx, Ptr<Packet> packet);

    virtual void SendHelloPacket(uint32_t ifaceIdx);
    virtual void SendDatabaseDescriptionPacket(uint32_t ifaceIdx, RouterId neighborRouterId = 0, bool isInit = false);
    virtual void SendLinkStateRequestPacket(uint32_t ifaceIdx, RouterId neighborRouterId = 0);
    virtual void SendLinkStateUpdatePacketEntryPoint();
    virtual void SendLinkStateUpdatePacket(uint32_t ifaceIdx);
    virtual void SendLinkStateUpdatePacket(uint32_t ifaceIdx, RouterId neighborRouterId);
    virtual void SendLinkStateUpdatePacketDirectAsap(uint32_t ifaceIdx, Ptr<OSPFLSA> lsa, RouterId neighborRouterId = 0);
    virtual void SendLinkStateUpdatePacketDirect(uint32_t ifaceIdx, std::vector<Ptr<OSPFLSA> >& lsas, RouterId neighborRouterId = 0);
    virtual void SendLinkStateAckPacket(uint32_t ifaceIdx, Ptr<OSPFLSAHeader> lsaHeader , RouterId neighborRouterId = 0);
    virtual void SendLinkStateAckPacket(uint32_t ifaceIdx, std::vector<Ptr<OSPFLSAHeader> >& lsaHeaders , RouterId neighborRouterId = 0);

    virtual void NotifyInterfaceEvent(uint32_t ifaceIdx, InterfaceEvent event);
    virtual void NotifyNeighborEvent(uint32_t ifaceIdx, RouterId neighborRouterId, NeighborEvent event);
    virtual bool IsNeighborToBeAdjacent(uint32_t ifaceIdx, RouterId neighborRouterId);
    virtual void RemoveFromAllRxmtList(OSPFLinkStateIdentifier& id);
    virtual Time& GetLastLSUSentTime ();
    virtual void AppendToRxmtList (Ptr<OSPFLSA> lsa, uint32_t ifaceIdx, RouterId senderRouterId, bool sendAsap = false);
    virtual void DirectAppendToRxmtList (Ptr<OSPFLSA> lsa, uint32_t ifaceIdx, RouterId neighborRouterId, bool sendAsap = false);

    virtual uint16_t CalcMetricForInterface (uint32_t ifaceIdx);

    virtual void OriginateRouterSpecificLSAs(uint32_t ifaceIdx, bool forceRefresh = false);
    virtual void OriginateLinkLSA(uint32_t ifaceIdx, bool forceRefresh = false);
    virtual void OriginateRouterLSA(bool forceRefresh = false);
    virtual void OriginateIntraAreaPrefixLSA(bool forceRefresh = false);

    virtual void CalcRoutingTable (bool recalcAll = false);
    virtual int32_t GetInterfaceForNeighbor (RouterId routerId);
    virtual void RegisterToLSDB (Ptr<OSPFLSA> lsa);
    virtual void UpdateLSACaches (Ptr<OSPFLSA> lsa);
    virtual void AddToRxmtList (int32_t ifaceIdx, Ptr<OSPFLSA> lsa);
    virtual void AddToRxmtList (int32_t ifaceIdx, RouterId neighborRouterId, Ptr<OSPFLSA> lsa);
    
    virtual void Start ();

    virtual Ptr<Ipv6Route> Lookup(Ipv6Address src, Ipv6Address dst, Ptr<NetDevice> interface = 0);

protected:
    virtual void DoDispose ();
};

} /* namespace ospf */
} /* namespace ns3 */

#endif /* IPV6_STATIC_ROUTING_H */

