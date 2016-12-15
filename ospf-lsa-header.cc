#include "ospf-lsa-header.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OSPFLSAHeader");
NS_OBJECT_ENSURE_REGISTERED(OSPFLSAHeader);

TypeId OSPFLSAHeader::GetTypeId () {
    static TypeId tid = TypeId("ns3::OSPFLSAHeader")
        .AddConstructor<OSPFLSAHeader>();
    return tid;
}

uint32_t OSPFLSAHeader::GetSerializedSize () const {
    return 20;
} 
void OSPFLSAHeader::Print (std::ostream &os) const {
    os << "age:" << m_age << ", ";
    os << "type:" << m_type << ", ";
    os << "id:" << m_id << ", ";
    os << "advRtr:" << m_advRtr << ", ";
    os << "seqNum:" << m_seqNum << ", ";
    os << "checksum:" << m_checksum << ", ";
    os << "length:" << m_length << ", ";
} 
void OSPFLSAHeader::Serialize (TagBuffer i) const {
    i.WriteU16(m_age);
    i.WriteU16(m_type);
    i.WriteU32(m_id);
    i.WriteU32(m_advRtr);
    i.WriteU32(m_seqNum);
    i.WriteU16(m_checksum);
    i.WriteU16(m_length);
}
uint32_t OSPFLSAHeader::Deserialize (TagBuffer i) {
    m_age = i.ReadU16();
    m_type = i.ReadU16();
    m_id = i.ReadU32();
    m_advRtr = i.ReadU32();
    m_seqNum = i.ReadU32();
    m_checksum = i.ReadU16();
    m_length = i.ReadU16();
    return 20;
}

} // namespace ns3
