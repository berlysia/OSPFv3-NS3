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
    return 1 + 1 + 2 + 4 + 4 + 2 + 1 + 1;
} 
void OSPFHeader::Print (std::ostream &os) const {
    os << "version: " << m_version << ", ";
    os << "type   : " << m_type << ", ";
    os << "router : " << m_routerId << ", ";
    os << "area   : " << m_areaId;
} 
void OSPFHeader::Serialize (Buffer::Iterator start) const {
    i.WriteHtonU8(m_version);
    i.WriteHtonU8(m_type);
    i.WriteHtonU16(m_packetLength);
    i.WriteHtonU32(m_routerId);
    i.WriteHtonU32(m_areaId);
    i.WriteHtonU16(m_checksum);
    i.WriteHtonU8(m_instanceId);
    i.WriteHtonU8(m_padding);
}
uint32_t OSPFHeader::Deserialize (Buffer::Iterator start) {
    m_version = i.ReadNtohU8();
    m_type = i.ReadNtohU8();
    m_packetLength = i.ReadNtohU16();
    m_routerId = i.ReadNtohU32();
    m_areaId = i.ReadNtohU32();
    m_checksum = i.ReadNtohU16();
    m_instanceId = i.ReadNtohU8();
    m_padding = i.ReadNtohU8();
    return 1 + 1 + 2 + 4 + 4 + 2 + 1 + 1;
}

} // namespace ns3
