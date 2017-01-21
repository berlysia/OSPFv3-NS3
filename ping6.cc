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
#include <algorithm>
#include "ns3/ipv6-address.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ipv6-ospf-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Ping6Example");

#if 1
int main (int argc, char **argv)
{
  bool verbose = false;
  int nodes = 2;
  std::string inputFile = "";

  CommandLine cmd;
  cmd.AddValue ("verbose", "turn on log components", verbose);
  cmd.AddValue ("inputFile", "input file name", inputFile);
  /*
    # input file format (1-indexed)

    [nodes]
    [conns]
    [src] [dst] [dataRate]
    [src] [dst] [dataRate]
    ...
    [ping pairs]
    [src] [dst] [packets] [interval(milliseconds)] [size]
  */
  cmd.AddValue ("nodes", "number of nodes", nodes);
  cmd.Parse (argc, argv);

  int conns = nodes - 1;
  int pingPairs = 1;
  std::vector<int> connSrc, connDst, connDataRate;
  std::vector<int> pingSrc, pingDst, hasPings, pingToNode, pingPackets, pingIntervals, pingSizes, pingStart, pingEnd;
  
  if (verbose) {
    // LogComponentEnable ("Ipv6Address", LOG_LEVEL_ALL);
    // LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_ALL);
    // LogComponentEnable ("Ping6Application", LOG_LEVEL_ALL);
    LogComponentEnable ("Ping6Example", LOG_LEVEL_ALL);
    // LogComponentEnable ("RoutingTable", LOG_LEVEL_ALL);
    LogComponentEnable ("Ipv6OspfRouting", LOG_LEVEL_ALL);
    // LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
    // LogComponentEnable ("Ipv6RawSocketImpl", LOG_LEVEL_ALL);
  }

  if (inputFile != "") {
    NS_LOG_INFO("ファイルから読み込みます: " << inputFile);
    std::ifstream ifs(inputFile);
    if (ifs.fail()) {
      NS_LOG_ERROR("ファイル: " << inputFile << " を開けませんでした");
      return 1;
    }

    ifs >> nodes;
    ifs >> conns;
    connSrc.resize(conns);
    connDst.resize(conns);
    connDataRate.resize(conns);
    for (int i = 0, l = conns; i < l; ++i) {
      ifs >> connSrc[i] >> connDst[i] >> connDataRate[i];
      NS_LOG_INFO("read conn: src " << connSrc[i] << ", dst " << connDst[i] << ", rate " << connDataRate[i]);
      --connSrc[i];
      --connDst[i];
    }
    ifs >> pingPairs;
    pingSrc.resize(pingPairs);
    pingDst.resize(pingPairs);
    pingPackets.resize(pingPairs);
    pingIntervals.resize(pingPairs);
    pingSizes.resize(pingPairs);
    pingStart.resize(pingPairs);
    pingEnd.resize(pingPairs);
    for (int i = 0; i < pingPairs; ++i) {
      ifs >> pingSrc[i] >> pingDst[i] >> pingPackets[i] >> pingIntervals[i] >> pingSizes[i] >> pingStart[i] >> pingEnd[i];
      NS_LOG_INFO("read ping: src " << pingSrc[i] << ", dst " << pingDst[i] << ", Packets " << pingPackets[i] << ", Intervals " << pingIntervals[i] << ", Sizes " << pingSizes[i]);
      --pingSrc[i];
      --pingDst[i];
    }
    ifs.close();

    NS_LOG_INFO("ファイルから読み込みました:");
  } else {
    NS_LOG_INFO("ノード数から自動生成します: ノード数 " << nodes);
    connSrc.resize(conns);
    connDst.resize(conns);
    connDataRate.resize(conns);
    for (int i = 0, l = conns; i < l; ++i) {
      connSrc[i] = i;
      connDst[i] = i + 1;
      connDataRate[i] = 5000000;
    }
    pingSrc.push_back(connSrc[0]);
    pingDst.push_back(connDst.back());
  }

  NS_LOG_INFO("  ノード数: " << nodes);
  NS_LOG_INFO("  エッジ数: " << conns);
  for (int i = 0, l = conns; i < l; ++i) {
    NS_LOG_INFO("    " << connSrc[i]+1 << " <- [" << connDataRate[i] << "] -> " << connDst[i]+1);
  }
  for (int i = 0, l = pingPairs; i < l; ++i) {
    NS_LOG_INFO("    " << pingSrc[i]+1 << " --> " << pingDst[i]+1);
  }

  std::copy(pingSrc.begin(), pingSrc.end(), std::back_inserter(hasPings));
  std::copy(pingDst.begin(), pingDst.end(), std::back_inserter(hasPings));
  std::sort(hasPings.begin(), hasPings.end());
  hasPings.erase(std::unique(hasPings.begin(), hasPings.end()), hasPings.end());

  NS_LOG_INFO ("Create nodes.");
  NodeContainer ns, uns;
  ns.Create(nodes + hasPings.size());

  // create p2p conns
  std::vector<NodeContainer> nc;
  for (int i = 0, l = conns; i < l; ++i) {
    nc.push_back(NodeContainer(ns.Get(connSrc[i]), ns.Get(connDst[i])));
  }
  
  // create user network
  std::vector<NodeContainer> unc;
  for (int i = 0, l = hasPings.size(); i < l; ++i) {
    unc.push_back(NodeContainer(ns.Get(hasPings[i]), ns.Get(nodes + i)));
  }

  pingToNode.resize(nodes+1);
  for (int i = 0, l = hasPings.size(); i < l; ++i) {
    pingToNode[hasPings[i]] = i;
  }

  /* Install IPv4/IPv6 stack */
  InternetStackHelper internetv6;
  internetv6.SetIpv4StackInstall (false);
  internetv6.Install (ns);

  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));
  std::vector<NetDeviceContainer> devs, pdevs;
  for (int i = 0, l = conns; i < l; ++i) {
    p2p.SetDeviceAttribute ("DataRate", DataRateValue (connDataRate[i]));
    devs.push_back(p2p.Install(nc[i]));
  }

  p2p.SetDeviceAttribute ("DataRate", DataRateValue (1000000000));
  for (int i = 0, l = hasPings.size(); i < l; ++i) {
    pdevs.push_back(p2p.Install(unc[i]));
  }

  uint8_t addrBuf[16] = {0x20, 0x01, 0xca, 0xfe, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  Ipv6AddressHelper ipv6;
  NS_LOG_INFO ("Assign IPv6 Addresses. base addr: " << Ipv6Address(addrBuf));

  std::vector<Ipv6InterfaceContainer> ifaces, pifaces;
  for (int i = 0, l = conns; i < l; ++i) {
    addrBuf[5] = i + 1;
    NS_LOG_INFO ("Assign IPv6 Address for network #" << i << " : " << Ipv6Address(addrBuf));
    ipv6.SetBase(Ipv6Address(addrBuf), Ipv6Prefix(64));
    ifaces.push_back(ipv6.Assign(devs[i]));
    ifaces[i].SetForwarding(0, true);
    ifaces[i].SetForwarding(1, true);
  }

  addrBuf[2] = 0xbe;
  addrBuf[3] = 0xef;
  for (int i = 0, l = hasPings.size(); i < l; ++i) {
    addrBuf[5] = i + 1;
    NS_LOG_INFO ("Assign IPv6 Address for user network #" << i << " : " << Ipv6Address(addrBuf));
    ipv6.SetBase(Ipv6Address(addrBuf), Ipv6Prefix(64));
    pifaces.push_back(ipv6.Assign(pdevs[i]));
    pifaces[i].SetForwarding(0, true);
  }

  NS_LOG_INFO ("Apply Routings.");
#if 1
  ospf::Ipv6OspfRoutingHelper ipv6RoutingHelper;
  for (int i = 0, l = nodes; i < l; ++i) {
    ns.Get(i)->GetObject<Ipv6>()->SetRoutingProtocol(CreateObject<ospf::Ipv6OspfRouting>());
  }
  Ipv6StaticRoutingHelper staticRoutingHelper;
  Ptr<Ipv6StaticRouting> rt;
  for (int i = 0, l = hasPings.size(); i < l; ++i) {
    rt = staticRoutingHelper.GetStaticRouting(ns.Get(nodes + i)->GetObject<Ipv6>());
    rt->SetDefaultRoute(pifaces[i].GetAddress (0, 1), 1);
    // rt->AddNetworkRouteTo(Ipv6Address::GetZero(), Ipv6Prefix::GetZero(), 1);
    // NS_LOG_INFO("iface: " << ns.Get(nodes + i)->GetObject<Ipv6>()->GetInterfaceForAddress (pifaces[i].GetAddress (1, 1)));
  }
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
  // ping6.SetIfIndex (i.GetInterfaceIndex (0));
  // ping6.SetRemote (Ipv6Address::GetAllNodesMulticast ());


  for (int i = 0, l = pingSrc.size(); i < l; ++i) {
    NS_LOG_INFO("Install Ping6 to Node #"<<pingSrc[i]<<" : " << pifaces[pingToNode[pingSrc[i]]].GetAddress (1, 1) << " -> " << pifaces[pingToNode[pingDst[i]]].GetAddress (1, 1));
    ping6.SetAttribute ("MaxPackets", UintegerValue (pingPackets[i]));
    ping6.SetAttribute ("Interval", TimeValue (MicroSeconds(pingIntervals[i])));
    ping6.SetAttribute ("PacketSize", UintegerValue (pingSizes[i]));
    ping6.SetLocal (pifaces[pingToNode[pingSrc[i]]].GetAddress (1, 1)); 
    ping6.SetRemote (pifaces[pingToNode[pingDst[i]]].GetAddress (1, 1));
    ApplicationContainer apps = ping6.Install (ns.Get (nodes + pingToNode[pingSrc[i]]));
    apps.Start (Seconds (pingStart[i]));
    apps.Stop (Seconds (pingEnd[i]));
  }

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("ping6.tr"));
  p2p.EnablePcapAll (std::string ("ping6"), true);

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
  if (pingPairs > 0) {
    int firstPingStart = *std::min_element(pingStart.begin(), pingStart.end());
    int lastPingEnd = *std::max_element(pingEnd.begin(), pingEnd.end());
    for (int i = 0, l = nodes; i < l; ++i) {
      ipv6RoutingHelper.PrintRoutingTableAt (Seconds (firstPingStart), ns.Get(i), routingStream);
      ipv6RoutingHelper.PrintRoutingTableAt (Seconds (lastPingEnd), ns.Get(i), routingStream);
    }
  } else {
    for (int i = 0, l = nodes; i < l; ++i) {
      ipv6RoutingHelper.PrintRoutingTableAt (Seconds (10), ns.Get(i), routingStream);
      ipv6RoutingHelper.PrintRoutingTableAt (Seconds (250), ns.Get(i), routingStream);
    }
  }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds(300));
  Simulator::Run ();
  // Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}

#endif