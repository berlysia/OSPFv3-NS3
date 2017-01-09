#include "ospf-header.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/buffer.h"

#include <iostream>

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE("OSPFHeader");
NS_OBJECT_ENSURE_REGISTERED(OSPFHeader);

TypeId OSPFHeader::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFHeader")
        .SetParent<Header>()
        .AddConstructor<OSPFHeader>();
    return tid;
}

TypeId OSPFHeader::GetInstanceTypeId () const {
    return GetTypeId();
}

uint32_t OSPFHeader::GetSerializedSize () const {
    return 19;
} 
void OSPFHeader::Print (std::ostream &os) const {
    os << "# OSPFHeader\n";
    os << "version: " << (int)m_version << "\n";
    os << "type   : " << (int)m_type << "\n";
    os << "packetLength   : " << m_packetLength << "\n";
    os << "router : " << m_routerId << "\n";
    os << "area   : " << m_areaId << "\n";
    os << "checksum   : " << m_checksum << "\n";
    os << "instanceId   : " << (int)m_instanceId << "\n";
    os << "interfaceId   : " << (int)m_interfaceId << "\n";
} 
void OSPFHeader::Serialize (Buffer::Iterator start) const {
    start.WriteU8(m_version);
    start.WriteU8(m_type);
    start.WriteHtonU16(m_packetLength);
    start.WriteHtonU32(m_routerId);
    start.WriteHtonU32(m_areaId);
    start.WriteHtonU16(m_checksum);
    start.WriteU8(m_instanceId);
    start.WriteHtonU32(m_interfaceId);
}
uint32_t OSPFHeader::Deserialize (Buffer::Iterator start) {
    m_version = start.ReadU8();
    m_type = start.ReadU8();
    m_packetLength = start.ReadNtohU16();
    m_routerId = start.ReadNtohU32();
    m_areaId = start.ReadNtohU32();
    m_checksum = start.ReadNtohU16();
    m_instanceId = start.ReadU8();
    m_interfaceId = start.ReadNtohU32();
    return OSPFHeader::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
