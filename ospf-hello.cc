#include "ospf-hello.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE("OSPFHello");
NS_OBJECT_ENSURE_REGISTERED(OSPFHello);

TypeId OSPFHello::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFHello")
        .SetParent<OSPFHeader>()
        .AddConstructor<OSPFHello>();
    return tid;
}

TypeId OSPFHello::GetInstanceId () const {
    return GetTypeId();
}

uint32_t OSPFHello::GetSerializedSize () const {
    return OSPFHeader::GetSerializedSize() + 4 + 1 + 2 + 2 + 4 + 4 + 4 + m_neighborId.size() * 4;
} 
void OSPFHello::Print (std::ostream &os) const {
    os << "(";
    OSPFHeader::Print(os);
    os << " Hello - ";
    os << "option: " << m_options << ", ";
    os << "routerPriority: " << (int)m_routerPriority << ", ";
    os << "helloInterval: " << m_helloInterval << ", ";
    os << "deadInterval : " << m_routerDeadInterval << ", ";
    os << "DRouter  : " << m_designatedRouterId << ", ";
    os << "BDRouter : " << m_backupDesignatedRouterId << ", ";
    os << "neighbors: " << m_neighborId.size() << ", ";
    os << "[";
    for (int i = 0, l = m_neighborId.size(); i < l; ++i) {
        os << m_neighborId[i] << ", ";
    }
    os << "])";
} 
void OSPFHello::Serialize (Buffer::Iterator start) const {
    OSPFHeader::Serialize(start);
    start.Next(OSPFHeader::GetSerializedSize());

    start.WriteHtonU32(m_options);
    start.WriteU8(m_routerPriority);
    start.WriteHtonU16(m_helloInterval);
    start.WriteHtonU16(m_routerDeadInterval);
    start.WriteHtonU32(m_designatedRouterId);
    start.WriteHtonU32(m_backupDesignatedRouterId);
    uint32_t size = m_neighborId.size();
    start.WriteHtonU32(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        start.WriteHtonU32(m_neighborId[idx]);
    }
}
uint32_t OSPFHello::Deserialize (Buffer::Iterator start) {
    
    start.Next(OSPFHeader::Deserialize(start));

    m_options = start.ReadNtohU32();
    m_routerPriority = start.ReadU8();
    m_helloInterval = start.ReadNtohU16();
    m_routerDeadInterval = start.ReadNtohU16();
    m_designatedRouterId = start.ReadNtohU32();
    m_backupDesignatedRouterId = start.ReadNtohU32();
    uint32_t size = start.ReadNtohU32();
    m_neighborId.resize(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        m_neighborId[idx] = start.ReadNtohU32();
    }

    return OSPFHello::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
