/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008-2009 Strasbourg University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 */

// Network topology
//
//       n0    n1
//       |     |
//       =================
//              LAN
//
// - ICMPv6 echo request flows from n0 to n1 and back with ICMPv6 echo reply
// - DropTail queues 
// - Tracing of queues and packet receptions to file "ping6.tr"

#include <fstream>
#include <iostream>
#include <vector>
#include "ns3/ipv6-address.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ipv6-ospf-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Ping6Example");

int main (int argc, char **argv)
{
  bool verbose = false;

  CommandLine cmd;
  cmd.AddValue ("verbose", "turn on log components", verbose);
  cmd.Parse (argc, argv);

  // LogComponentEnable ("Ping6Example", LOG_LEVEL_ALL);
  LogComponentEnable ("Ipv6OspfRouting", LOG_LEVEL_ALL);
  // LogComponentEnable ("Ipv6RawSocketImpl", LOG_LEVEL_ALL);
  LogComponentEnable ("RoutingTable", LOG_LEVEL_ALL);
  // LogComponentEnable ("Packet", LOG_LEVEL_ALL);
  // LogComponentEnable ("Ipv6Header", LOG_LEVEL_ALL);
  // LogComponentEnable ("Ipv6Address", LOG_LEVEL_ALL);
  // LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
  // LogComponentEnable ("Ipv6RoutingProtocol", LOG_LEVEL_ALL);
  if (verbose)
    {
      LogComponentEnable ("Ping6Example", LOG_LEVEL_INFO);
      LogComponentEnable ("Ipv6EndPointDemux", LOG_LEVEL_ALL);
      LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_ALL);
      LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
      LogComponentEnable ("Ipv6ListRouting", LOG_LEVEL_ALL);
      LogComponentEnable ("Ipv6Interface", LOG_LEVEL_ALL);
      LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_ALL);
      LogComponentEnable ("Ping6Application", LOG_LEVEL_ALL);
      LogComponentEnable ("NdiscCache", LOG_LEVEL_ALL);
    }

  NS_LOG_INFO ("Create nodes.");
  NodeContainer ns;
  ns.Create(4);

  // create p2p connections
  NodeContainer nc01(ns.Get(0), ns.Get(1)), nc12(ns.Get(1), ns.Get(2)), nc23(ns.Get(2), ns.Get(3));

  /* Install IPv4/IPv6 stack */
  InternetStackHelper internetv6;
  internetv6.SetIpv4StackInstall (false);
  internetv6.Install (ns);

  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (5000000));
  p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer d01 = p2p.Install (nc01);
  NetDeviceContainer d12 = p2p.Install (nc12);
  NetDeviceContainer d23 = p2p.Install (nc23);

  Ipv6AddressHelper ipv6;
  NS_LOG_INFO ("Assign IPv6 Addresses.");
  ipv6.SetBase(Ipv6Address("2001:cafe:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer i01 = ipv6.Assign (d01);
  i01.SetForwarding(1, true);
  ipv6.SetBase(Ipv6Address("2001:cafe:2::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer i12 = ipv6.Assign (d12);
  i12.SetForwarding(0, true);
  i12.SetForwarding(1, true);
  ipv6.SetBase(Ipv6Address("2001:cafe:3::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer i23 = ipv6.Assign (d23);
  i23.SetForwarding(0, true);

  NS_LOG_INFO ("Apply Routings.");
#if 1
  ospf::Ipv6OspfRoutingHelper ipv6RoutingHelper;
  ns.Get(0)->GetObject<Ipv6>()->SetRoutingProtocol(CreateObject<ospf::Ipv6OspfRouting>());
  ns.Get(1)->GetObject<Ipv6>()->SetRoutingProtocol(CreateObject<ospf::Ipv6OspfRouting>());
  ns.Get(2)->GetObject<Ipv6>()->SetRoutingProtocol(CreateObject<ospf::Ipv6OspfRouting>());
  ns.Get(3)->GetObject<Ipv6>()->SetRoutingProtocol(CreateObject<ospf::Ipv6OspfRouting>());
#else
  Ipv6StaticRoutingHelper ipv6RoutingHelper;
  Ptr<Ipv6StaticRouting> rt0 = ipv6RoutingHelper.GetStaticRouting(ns.Get(0)->GetObject<Ipv6>());
  rt0->AddNetworkRouteTo(Ipv6Address("2001:cafe:2::"), Ipv6Prefix(64), 1);
  rt0->AddNetworkRouteTo(Ipv6Address("2001:cafe:3::"), Ipv6Prefix(64), 1);
  Ptr<Ipv6StaticRouting> rt1 = ipv6RoutingHelper.GetStaticRouting(ns.Get(1)->GetObject<Ipv6>());
  rt1->AddNetworkRouteTo(Ipv6Address("2001:cafe:3::"), Ipv6Prefix(64), 2);
  Ptr<Ipv6StaticRouting> rt2 = ipv6RoutingHelper.GetStaticRouting(ns.Get(2)->GetObject<Ipv6>());
  rt2->AddNetworkRouteTo(Ipv6Address("2001:cafe:1::"), Ipv6Prefix(64), 1);
  Ptr<Ipv6StaticRouting> rt3 = ipv6RoutingHelper.GetStaticRouting(ns.Get(3)->GetObject<Ipv6>());
  rt3->AddNetworkRouteTo(Ipv6Address("2001:cafe:1::"), Ipv6Prefix(64), 1);
  rt3->AddNetworkRouteTo(Ipv6Address("2001:cafe:2::"), Ipv6Prefix(64), 1);
#endif

  NS_LOG_INFO ("Create Applications.");

  /* Create a Ping6 application to send ICMPv6 echo request from node zero to
   * all-nodes (ff02::1).
   */
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 5;
  Time interPacketInterval = Seconds (1.);
  Ping6Helper ping6;

  ping6.SetLocal (i01.GetAddress (0, 1)); 
  // ping6.SetRemote (i01.GetAddress (1, 1));
  // ping6.SetRemote (i12.GetAddress (0, 1));
  ping6.SetRemote (i23.GetAddress (1, 1));
  // ping6.SetIfIndex (i.GetInterfaceIndex (0));
  // ping6.SetRemote (Ipv6Address::GetAllNodesMulticast ());

  ping6.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  ping6.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping6.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps = ping6.Install (ns.Get (0));
  apps.Start (Seconds (20.0));
  apps.Stop (Seconds (60.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("ping6.tr"));
  p2p.EnablePcapAll (std::string ("ping6"), true);

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
  ipv6RoutingHelper.PrintRoutingTableAt (Seconds (19.9), ns.Get(0), routingStream);
  ipv6RoutingHelper.PrintRoutingTableAt (Seconds (19.9), ns.Get(1), routingStream);
  ipv6RoutingHelper.PrintRoutingTableAt (Seconds (19.9), ns.Get(2), routingStream);
  ipv6RoutingHelper.PrintRoutingTableAt (Seconds (19.9), ns.Get(3), routingStream);
  ipv6RoutingHelper.PrintRoutingTableAt (Seconds (60.1), ns.Get(0), routingStream);
  ipv6RoutingHelper.PrintRoutingTableAt (Seconds (60.1), ns.Get(1), routingStream);
  ipv6RoutingHelper.PrintRoutingTableAt (Seconds (60.1), ns.Get(2), routingStream);
  ipv6RoutingHelper.PrintRoutingTableAt (Seconds (60.1), ns.Get(3), routingStream);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds(62));
  Simulator::Run ();
  // Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}

