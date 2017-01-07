#include "ospf-header.h"
#include "ns3/test.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include <iostream>
using namespace std;

void TestForOSPFHeader () {

    cout << " - TestForOSPFHeader - " << endl;
    ns3::ospf::OSPFHeader ospfHeader;
    ospfHeader.SetVersion(3);
    ospfHeader.SetType(6);
    ospfHeader.SetAreaId(234);
    ospfHeader.SetRouterId(123);
    ospfHeader.SetInstanceId(5);
    // ospfHeader.Print(cout);
    // cout << "---" << endl;

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(ospfHeader);
    // packet->Print(cout);
    // cout << "---" << endl;

    ns3::ospf::OSPFHeader deserialized;
    packet->RemoveHeader(deserialized);
    // deserialized.Print(cout);

    NS_ASSERT(ospfHeader == deserialized);

    return;
}