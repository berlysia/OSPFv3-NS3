#include "ospf-lsa-header.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE("OSPFLSAHeader");
NS_OBJECT_ENSURE_REGISTERED(OSPFLSAHeader);

TypeId OSPFLSAHeader::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFLSAHeader")
        .AddConstructor<OSPFLSAHeader>();
    return tid;
}

TypeId OSPFLSAHeader::GetInstanceTypeId () const {
    return GetTypeId();
}

uint32_t OSPFLSAHeader::GetSerializedSize () const {
    return 20;
} 
void OSPFLSAHeader::Print (std::ostream &os) const {
    os << "(LSAHeader: [";
    os << "age:" << m_age << ", ";
    os << "type:" << m_type << ", ";
    os << "id:" << m_id << ", ";
    os << "advRtr:" << m_advRtr << ", ";
    os << "seqNum:" << m_seqNum << ", ";
    // os << "checksum:" << m_checksum << ", ";
    // os << "length:" << m_length;
    os << " ])";
} 
void OSPFLSAHeader::Serialize (Buffer::Iterator &i) const {
    i.WriteHtonU16(m_age);
    i.WriteHtonU16(m_type);
    i.WriteHtonU32(m_id);
    i.WriteHtonU32(m_advRtr);
    i.WriteHtonU32(m_seqNum);
    i.WriteHtonU16(m_checksum);
    i.WriteHtonU16(m_length);
}
uint32_t OSPFLSAHeader::Deserialize (Buffer::Iterator &i) {
    m_age = i.ReadNtohU16();
    m_type = i.ReadNtohU16();
    m_id = i.ReadNtohU32();
    m_advRtr = i.ReadNtohU32();
    m_seqNum = i.ReadNtohU32();
    m_checksum = i.ReadNtohU16();
    m_length = i.ReadNtohU16();
    return OSPFLSAHeader::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
