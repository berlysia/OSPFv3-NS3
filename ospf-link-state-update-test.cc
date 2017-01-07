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
    
    ns3::ospf::OSPFLSA h1, h2, h3;
    h1.Initialize(OSPF_LSA_TYPE_LINK);
    h2.Initialize(OSPF_LSA_TYPE_LINK);
    h3.Initialize(OSPF_LSA_TYPE_ROUTER);
    srcHdr.SetLSAHeader(h1);
    srcHdr.SetLSAHeader(h2);
    srcHdr.SetLSAHeader(h3);


    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(srcHdr);

    packet->RemoveHeader(dstHdr);

    // srcHdr.Print(cout);
    // cout << "#### ---- #### ---- ####\n";
    // dstHdr.Print(cout);

    NS_ASSERT(srcHdr == dstHdr);

    return;
}