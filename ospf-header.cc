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
    return 16;
} 
void OSPFHeader::Print (std::ostream &os) const {
    os << "(";
    os << "version: " << (int)m_version << ", ";
    os << "type: " << (int)m_type << ", ";
    // os << "packetLength: " << m_packetLength << ", ";
    os << "router: " << m_routerId << ", ";
    os << "area: " << m_areaId << ", ";
    // os << "checksum: " << m_checksum << ", ";
    os << "instanceId: " << (int)m_instanceId << ")";
} 
void OSPFHeader::Serialize (Buffer::Iterator start) const {
/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |      3        |       1       |         Packet Length         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                         Router ID                             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                          Area ID                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |          Checksum             | Instance ID   |     0         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
    start.WriteU8(m_version);
    start.WriteU8(m_type);
    start.WriteHtonU16(GetSerializedSize());
    start.WriteHtonU32(m_routerId);
    start.WriteHtonU32(m_areaId);
    start.WriteHtonU16(0);
    start.WriteU8(m_instanceId);
    start.WriteU8(0);
}
uint32_t OSPFHeader::Deserialize (Buffer::Iterator start) {
    m_version = start.ReadU8();
    m_type = start.ReadU8();
    m_packetLength = start.ReadNtohU16();
    m_routerId = start.ReadNtohU32();
    m_areaId = start.ReadNtohU32();
    start.ReadNtohU16();
    m_instanceId = start.ReadU8();
    start.ReadU8();
    return OSPFHeader::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
