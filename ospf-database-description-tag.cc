#include "ospf-database-description-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OSPFDatabaseDescriptionTag");
NS_OBJECT_ENSURE_REGISTERED(OSPFDatabaseDescriptionTag);

TypeId OSPFDatabaseDescriptionTag::GetTypeId () {
    static TypeId tid = TypeId("ns3::OSPFDatabaseDescriptionTag")
        .SetParent<Tag>()
        .AddConstructor<OSPFDatabaseDescriptionTag>();
    return tid;
}

/*
uint24_t m_options;
uint16_t m_mtu;
bool m_initFlag;
bool m_moreFlag;
bool m_masterFlag;
uint32_t m_ddSeqNum;
vector<OSPFLSAHeader> m_lsaHeaders;
*/

uint32_t OSPFDatabaseDescriptionTag::GetSerializedSize () const {
    return 3 + 2 + 1 + 4 + m_lsaHeaders.size() * 20;
} 
void OSPFDatabaseDescriptionTag::Print (std::ostream &os) const {
    os << "options  : " << m_options << ", ";
    os << "MTU      : " << m_mtu << ", ";
    os << "isInit   : " << m_initFlag << ", ";
    os << "hasMore  : " << m_moreFlag << ", ";
    os << "isMaster : " << m_masterFlag << ", ";
    os << "ddSeqNum : " << m_ddSeqNum << ", ";
    os << "lsaHeaders: " << m_lsaHeaders.size() << ", [ ";
    for (int i = 0, l = m_lsaHeaders.size(); i < l; ++i) {
        os << "(";
        m_lsaHeaders[i].Print(os);
        os << ")";
    }
    os << "]";
} 
void OSPFDatabaseDescriptionTag::Serialize (TagBuffer i) const {
    i.WriteU8((m_options >> 16) & 0xff);
    i.WriteU16(m_options & 0xffff);
    i.WriteU16(m_mtu);
    i.WriteU8(
        (m_initFlag << 2) |
        (m_moreFlag << 1) |
        m_masterFlag
    );
    i.WriteU32(m_ddSeqNum);
    for(intidxi = 0, l = m_lsaHeaders.size(); idx < l; ++idx) {
        m_lsaHeaders[idx].Serialize(i);
        i.Next(20);
    }
}
void OSPFDatabaseDescriptionTag::Deserialize (TagBuffer i) {
    m_options = (i.ReadU8() << 16) | i.ReadU16();
    m_mtu = i.ReadU16();
    uint8_t buff = i.ReadU8();
    m_initFlag = (buff >> 2) & 0x1;
    m_moreFlag = (buff >> 1) & 0x1;
    m_masterFlag = buff & 0x1;
    m_ddSeqNum = i.ReadU32();
    while(!i.IsEnd()) {
        OSPFLSAHeader h;
        i.Next(h.Deserialize(i));
        m_lsaHeaders.push_back(h);
    }
}

} // namespace ns3
