#include "ospf-link-state-request.h"
#include "ns3/test.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include <iostream>
using namespace std;

void TestForOSPFLinkStateRequest () {

    cout << " - TestForOSPFLinkStateRequest - " << endl;
    ns3::ospf::OSPFLinkStateRequest srcHdr, dstHdr;
    srcHdr.SetAreaId(234);
    srcHdr.SetRouterId(123);
    srcHdr.SetInstanceId(5);
    
    srcHdr.SetLinkStateIdentifier(12, 345, 678);
    srcHdr.SetLinkStateIdentifier(13, 456, 789);
    srcHdr.SetLinkStateIdentifier(14, 567, 890);
    srcHdr.SetLinkStateIdentifier(15, 678, 901);
    srcHdr.SetLinkStateIdentifier(16, 789, 12);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(srcHdr);

    packet->RemoveHeader(dstHdr);

    NS_ASSERT(srcHdr == dstHdr);

    return;
}