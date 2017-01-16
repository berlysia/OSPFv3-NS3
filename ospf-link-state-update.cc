#include "ospf-link-state-update.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE("OSPFLinkStateUpdate");
NS_OBJECT_ENSURE_REGISTERED(OSPFLinkStateUpdate);

TypeId OSPFLinkStateUpdate::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFLinkStateUpdate")
        .SetParent<OSPFHeader>()
        .AddConstructor<OSPFLinkStateUpdate>();
    return tid;
}

TypeId OSPFLinkStateUpdate::GetInstanceTypeId () const {
    return GetTypeId();
}

uint32_t OSPFLinkStateUpdate::GetSerializedSize () const {
    uint32_t size = 0;
    for (const OSPFLSA &lsa : m_lsas) {
        size += lsa.GetSerializedSize();
    }
    return OSPFHeader::GetSerializedSize() + 4 + size;
} 
void OSPFLinkStateUpdate::Print (std::ostream &os) const {
    os << "(";
    OSPFHeader::Print(os);
    os << "Link State Update";
    os << "lsas: " << m_lsas.size() << "(";
    for (int i = 0, l = m_lsas.size(); i < l; ++i) {
        m_lsas[i].Print(os);
        os << ", ";
    }
    os << ")";
} 
void OSPFLinkStateUpdate::Serialize (Buffer::Iterator start) const {
    OSPFHeader::Serialize(start);
    start.Next(OSPFHeader::GetSerializedSize());

    uint32_t size = m_lsas.size();
    start.WriteHtonU32(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        m_lsas[idx].Serialize(start);
        // start.Next(20);
    }
}
uint32_t OSPFLinkStateUpdate::Deserialize (Buffer::Iterator start) {
    start.Next(OSPFHeader::Deserialize(start));
    // OSPFHeader::Deserialize(start);
    
    uint32_t size = start.ReadNtohU32();
    m_lsas.resize(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        // start.Next(m_lsas[idx].Deserialize(start));
        m_lsas[idx].Deserialize(start);
    }

    return OSPFLinkStateUpdate::GetSerializedSize ();
}

} // namespace ns3
} // namespace ns3
