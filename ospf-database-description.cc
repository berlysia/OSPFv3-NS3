#include "ospf-database-description.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE("OSPFDatabaseDescription");
NS_OBJECT_ENSURE_REGISTERED(OSPFDatabaseDescription);

TypeId OSPFDatabaseDescription::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFDatabaseDescription")
        .SetParent<OSPFHeader>()
        .AddConstructor<OSPFDatabaseDescription>();
    return tid;
}

TypeId OSPFDatabaseDescription::GetInstanceTypeId () const {
    return GetTypeId();
}

/*
uint32_t m_options;
uint16_t m_mtu;
bool m_initFlag;
bool m_moreFlag;
bool m_masterFlag;
uint32_t m_ddSeqNum;
vector<OSPFLSAHeader> m_lsaHeaders;
*/

uint32_t OSPFDatabaseDescription::GetSerializedSize () const {
    return OSPFHeader::GetSerializedSize() + 4 + 2 + 1 + 4 + 4 + m_lsaHeaders.size() * 20;
} 
void OSPFDatabaseDescription::Print (std::ostream &os) const {
    OSPFHeader::Print(os);
    os << "## DatabaseDescription Packet \n";
    os << "  options  : " << m_options << "\n";
    os << "  MTU      : " << m_mtu << "\n";
    os << std::boolalpha;
    os << "  isInit   : " << m_initFlag << "\n";
    os << "  hasMore  : " << m_moreFlag << "\n";
    os << "  isMaster : " << m_masterFlag << "\n";
    os << std::noboolalpha;
    os << "  ddSeqNum : " << m_ddSeqNum << "\n";
    os << "  lsaHeaders: " << m_lsaHeaders.size() << "\n";
    for (int i = 0, l = m_lsaHeaders.size(); i < l; ++i) {
        m_lsaHeaders[i].Print(os);
    }
} 
void OSPFDatabaseDescription::Serialize (Buffer::Iterator start) const {
    OSPFHeader::Serialize(start);
    start.Next(OSPFHeader::GetSerializedSize());

    start.WriteHtonU32(m_options);
    start.WriteHtonU16(m_mtu);
    start.WriteU8(
        (m_initFlag << 2) |
        (m_moreFlag << 1) |
        m_masterFlag
    );
    start.WriteHtonU32(m_ddSeqNum);
    uint32_t size = m_lsaHeaders.size();
    start.WriteHtonU32(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        m_lsaHeaders[idx].Serialize(start);
        // start.Next(m_lsaHeaders[idx].GetSerializedSize());
        m_lsaHeaders[idx].GetSerializedSize();
    }
}
uint32_t OSPFDatabaseDescription::Deserialize (Buffer::Iterator start) {
    start.Next(OSPFHeader::Deserialize(start));

    m_options = start.ReadNtohU32();
    m_mtu = start.ReadNtohU16();
    uint8_t buff = start.ReadU8();
    m_initFlag = (buff >> 2) & 0x1;
    m_moreFlag = (buff >> 1) & 0x1;
    m_masterFlag = buff & 0x1;
    m_ddSeqNum = start.ReadNtohU32();
    
    uint32_t size = start.ReadNtohU32();
    m_lsaHeaders.resize(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        // start.Next(m_lsaHeaders[idx].Deserialize(start));
        m_lsaHeaders[idx].Deserialize(start);
    }

    return OSPFDatabaseDescription::GetSerializedSize ();
}

} // namespace ns3
} // namespace ns3
