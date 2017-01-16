#include "ospf-link-state-ack.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE("OSPFLinkStateAck");
NS_OBJECT_ENSURE_REGISTERED(OSPFLinkStateAck);

TypeId OSPFLinkStateAck::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFLinkStateAck")
        .SetParent<OSPFHeader>()
        .AddConstructor<OSPFLinkStateAck>();
    return tid;
}

TypeId OSPFLinkStateAck::GetInstanceTypeId () const {
    return GetTypeId();
}

uint32_t OSPFLinkStateAck::GetSerializedSize () const {
    return OSPFHeader::GetSerializedSize() + 4 + m_lsaHeaders.size() * 20;
} 
void OSPFLinkStateAck::Print (std::ostream &os) const {
    os << "(";
    OSPFHeader::Print(os);
    os << "Link State Ack - ";
    os << "lsaHeaders: " << m_lsaHeaders.size() << "[";
    for (int i = 0, l = m_lsaHeaders.size(); i < l; ++i) {
        m_lsaHeaders[i].Print(os);
        os << ", ";
    }
    os << "])";
} 
void OSPFLinkStateAck::Serialize (Buffer::Iterator start) const {
    OSPFHeader::Serialize(start);
    start.Next(OSPFHeader::GetSerializedSize());

    uint32_t size = m_lsaHeaders.size();
    start.WriteHtonU32(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        m_lsaHeaders[idx].Serialize(start);
        m_lsaHeaders[idx].GetSerializedSize();
    }
}
uint32_t OSPFLinkStateAck::Deserialize (Buffer::Iterator start) {
    start.Next(OSPFHeader::Deserialize(start));
    
    uint32_t size = start.ReadNtohU32();
    m_lsaHeaders.resize(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        m_lsaHeaders[idx].Deserialize(start);
    }

    return OSPFLinkStateAck::GetSerializedSize ();
}

} // namespace ns3
} // namespace ns3
