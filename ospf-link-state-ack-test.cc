#include "ospf-link-state-ack.h"
#include "ns3/test.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include <iostream>
using namespace std;

void TestForOSPFLinkStateAck () {

    cout << " - TestForOSPFLinkStateAck - " << endl;
    ns3::ospf::OSPFLinkStateAck srcHdr, dstHdr;
    srcHdr.SetAreaId(234);
    srcHdr.SetRouterId(123);
    srcHdr.SetInstanceId(5);
    
    srcHdr.SetLSAHeaders(vector<Ptr<ns3::ospf::OSPFLSAHeader>>(3));

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(srcHdr);

    packet->RemoveHeader(dstHdr);

    NS_ASSERT(srcHdr == dstHdr);

    return;
}