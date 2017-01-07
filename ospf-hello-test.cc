#include "ospf-hello.h"
#include "ns3/test.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include <iostream>
using namespace std;

void TestForOSPFHello () {

    cout << " - TestForOSPFHello - " << endl;
    ns3::ospf::OSPFHello srcHdr;
    srcHdr.SetAreaId(234);
    srcHdr.SetRouterId(123);
    srcHdr.SetInstanceId(5);
    srcHdr.SetRouterPriority(1);
    srcHdr.SetOptions(34567);
    srcHdr.SetHelloInterval(20);
    srcHdr.SetRouterDeadInterval(80);
    srcHdr.SetDesignatedRouter(114514);
    srcHdr.SetBackupDesignatedRouter(1919810);
    vector<uint32_t> ns(4, 100);
    srcHdr.SetNeighbors(ns);
    // srcHdr.Print(cout);
    // cout << "---" << endl;

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(srcHdr);
    // packet->Print(cout);
    // cout << "---" << endl;

    ns3::ospf::OSPFHello dstHdr;
    packet->RemoveHeader(dstHdr);
    // dstHdr.Print(cout);

    NS_ASSERT(srcHdr == dstHdr);

    return;
}