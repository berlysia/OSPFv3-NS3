#include "ospf-hello.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OSPFHello");
NS_OBJECT_ENSURE_REGISTERED(OSPFHello);

TypeId OSPFHello::GetTypeId () {
    static TypeId tid = TypeId("ns3::OSPFHello")
        .SetParent<Tag>()
        .AddConstructor<OSPFHello>();
    return tid;
}

uint32_t OSPFHello::GetSerializedSize () const {
    return OSPFHeader::GetSerializedSize() + 1 + 3 + 2 + 2 + 4 + 4 + m_neighbors.size() * 4;
} 
void OSPFHello::Print (std::ostream &os) const {
    OSPFHeader::Print(os);
    os << "routerPriority: " << m_routerPriority << "\n";
    os << "helloInterval: " << m_helloInterval << "\n";
    os << "deadInterval : " << m_routerDeadInterval << "\n";
    os << "DRouter  : " << m_designatedRouter << "\n";
    os << "BDRouter : " << m_backupDesinatedRouter << "\n";
    os << "neighbors: " << m_neighbors.size() << "\n";
    for (int i = 0, l = m_neighbors.size(); i < l; ++i) {
        os << "  " m_neighbors[i] << "\n"; 
    }
    os << "]";
} 
void OSPFHello::Serialize (Buffer::Iterator start) const {
    OSPFHeader::Serialize(start);
    start.Next(OSPFHeader::GetSerializedSize());

    start.WriteHton32((uint32_t)m_routerPriority << 24 | m_options);
    istart.WriteHtonU16(m_helloInterval);
    start.WriteHton16(m_routerDeadInterval);
    start.WriteHton32(m_designatedRouter);
    start.WriteHton32(m_backupDesinatedRouter);
    for(int idx = 0, l = m_neighbors.size(); idx < l; ++idx) {
        start.WriteHton32(m_neighbors[idx]);
    }
}
uint32_t OSPFHello::Deserialize (Buffer::Iterator start) {
    uint32_t pad = OSPFHeader::Deserialize(start);
    start.Next(pad);

    uint32_t buff;
    buff = start.ReadNtohU32();
    m_routerPriority = (buff & 0xe0000000) >> 24;
    m_options = buff & 0x1fffffff;
    m_helloInterval = start.ReadNtohU16();
    m_routerDeadInterval = start.ReadNtohU16();
    m_designatedRouter = start.ReadNtohU32();
    m_backupDesinatedRouter = start.ReadNtohU32();

    while(!i.IsEnd()) {
        m_neighbors.push_back(start.ReadNtohU32());
    }
}

} // namespace ns3
