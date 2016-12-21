#include "ospf-hello-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OSPFHelloTag");
NS_OBJECT_ENSURE_REGISTERED(OSPFHelloTag);

TypeId OSPFHelloTag::GetTypeId () {
    static TypeId tid = TypeId("ns3::OSPFHelloTag")
        .SetParent<Tag>()
        .AddConstructor<OSPFHelloTag>();
    return tid;
}

uint32_t OSPFHelloTag::GetSerializedSize () const {
    return 1 + 3 + 2 + 2 + 4 + 4 + m_neighbors.size() * 4;
} 
void OSPFHelloTag::Print (std::ostream &os) const {
    os << "routerPriority: " << m_routerPriority << ", ";
    os << "helloInterval: " << m_helloInterval << ", ";
    os << "deadInterval : " << m_routerDeadInterval << ", ";
    os << "DRouter  : " << m_designatedRouter << ", ";
    os << "BDRouter : " << m_backupDesinatedRouter << ", ";
    os << "neighbors: " << m_neighbors.size() << ", [ ";
    for (int i = 0, l = m_neighbors.size(); i < l; ++i) {
        os << m_neighbors[i] << ", "; 
    }
    os << "]";
} 
void OSPFHelloTag::Serialize (TagBuffer i) const {
    i.WriteU32((uint32_t)m_routerPriority << 24 | m_options);
    i.WriteU16(m_helloInterval);
    i.WriteU16(m_routerDeadInterval);
    i.WriteU32(m_designatedRouter);
    i.WriteU32(m_backupDesinatedRouter);
    for(int idx = 0, l = m_neighbors.size(); idx < l; ++idx) {
        i.WriteU32(m_neighbors[idx]);
    }
}
void OSPFHelloTag::Deserialize (TagBuffer i) {
    uint32_t buff;
    buff = i.ReadU32();
    m_routerPriority = (buff & 0xe0000000) >> 24;
    m_options = buff & 0x1fffffff;
    m_helloInterval = i.ReadU16();
    m_routerDeadInterval = i.ReadU16();
    m_designatedRouter = i.ReadU32();
    m_backupDesinatedRouter = i.ReadU32();

    while(!i.IsEnd()) {
        m_neighbors.push_back(i.ReadU32());
    }
}

} // namespace ns3
