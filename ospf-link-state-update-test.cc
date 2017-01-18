#include "ospf-link-state-update.h"
#include "ns3/test.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include <iostream>
using namespace std;

void TestForOSPFLinkStateUpdate () {

    cout << " - TestForOSPFLinkStateUpdate - " << endl;
    ns3::ospf::OSPFLinkStateUpdate srcHdr, dstHdr;
    srcHdr.SetAreaId(234);
    srcHdr.SetRouterId(123);
    srcHdr.SetInstanceId(5);
    
    Ptr<ns3::ospf::OSPFLSA> h1, h2, h3;
    h1 = Create<ns3::ospf::OSPFLSA>();
    h2 = Create<ns3::ospf::OSPFLSA>();
    h3 = Create<ns3::ospf::OSPFLSA>();
    h1->Initialize(OSPF_LSA_TYPE_LINK);
    h2->Initialize(OSPF_LSA_TYPE_LINK);
    h3->Initialize(OSPF_LSA_TYPE_ROUTER);
    srcHdr.AddLSA(h1);
    srcHdr.AddLSA(h2);
    srcHdr.AddLSA(h3);


    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(srcHdr);

    packet->RemoveHeader(dstHdr);

    // srcHdr.Print(cout);
    // cout << "#### ---- #### ---- ####\n";
    // dstHdr.Print(cout);

    NS_ASSERT(srcHdr == dstHdr);

    return;
}