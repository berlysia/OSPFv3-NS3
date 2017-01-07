#include "ospf-database-description.h"
#include "ns3/test.h"
#include "ns3/packet.h"
#include "ns3/packet-metadata.h"
#include "ns3/log.h"
#include <iostream>
using namespace std;

void TestForOSPFDatabaseDescription () {

    cout << " - TestForOSPFDatabaseDescription - " << endl;
    ns3::ospf::OSPFDatabaseDescription srcHdr, dstHdr;
    srcHdr.SetAreaId(234);
    srcHdr.SetRouterId(123);
    srcHdr.SetInstanceId(5);
    
    srcHdr.SetOptions(114514);
    srcHdr.SetMtu(567);
    srcHdr.SetInitFlag(true);
    srcHdr.SetMoreFlag(false);
    srcHdr.SetMasterFlag(true);
    srcHdr.SetSequenceNumber(6789);
    srcHdr.SetLSAHeaders(vector<ns3::ospf::OSPFLSAHeader>(3));

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(srcHdr);

    packet->RemoveHeader(dstHdr);

    NS_ASSERT(srcHdr == dstHdr);

    return;
}