#include "ospf-header.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/buffer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OSPFHeader");
NS_OBJECT_ENSURE_REGISTERED(OSPFHeader);

TypeId OSPFHeader::GetTypeId () {
    static TypeId tid = TypeId("ns3::OSPFHeader")
        .SetParent<Tag>()
        .AddConstructor<OSPFHeader>();
    return tid;
}

uint32_t OSPFHeader::GetSerializedSize () const {
    return 16;
} 
void OSPFHeader::Print (std::ostream &os) const {
    os << "version: " << m_version << "\n";
    os << "type   : " << m_type << "\n";
    os << "router : " << m_routerId << "\n";
    os << "area   : " << m_areaId << "\n";
} 
void OSPFHeader::Serialize (Buffer::Iterator start) const {
    start.WriteHtonU8(m_version);
    start.WriteHtonU8(m_type);
    start.WriteHtonU16(m_packetLength);
    start.WriteHtonU32(m_routerId);
    start.WriteHtonU32(m_areaId);
    start.WriteHtonU16(m_checksum);
    start.WriteHtonU8(m_instanceId);
    start.WriteHtonU8(m_padding);
}
uint32_t OSPFHeader::Deserialize (Buffer::Iterator start) {
    m_version = start.ReadNtohU8();
    m_type = start.ReadNtohU8();
    m_packetLength = start.ReadNtohU16();
    m_routerId = start.ReadNtohU32();
    m_areaId = start.ReadNtohU32();
    m_checksum = start.ReadNtohU16();
    m_instanceId = start.ReadNtohU8();
    m_padding = start.ReadNtohU8();
    return GetSerializedSize ();
}

} // namespace ns3
