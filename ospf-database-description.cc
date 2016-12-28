#include "ospf-database-description.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OSPFDatabaseDescriptionHeader");
NS_OBJECT_ENSURE_REGISTERED(OSPFDatabaseDescriptionHeader);

TypeId OSPFDatabaseDescriptionHeader::GetTypeId () {
    static TypeId tid = TypeId("ns3::OSPFDatabaseDescriptionHeader")
        .SetParent<Tag>()
        .AddConstructor<OSPFDatabaseDescriptionHeader>();
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

uint32_t OSPFDatabaseDescriptionHeader::GetSerializedSize () const {
    return OSPFHeader::GetSerializedSize() + 3 + 2 + 1 + 4 + m_lsaHeaders.size() * 20;
} 
void OSPFDatabaseDescriptionHeader::Print (std::ostream &os) const {
    OSPFHeader::Print(os);
    os << "DatabaseDescription >> \n";
    os << "options  : " << m_options << "\n";
    os << "MTU      : " << m_mtu << "\n";
    os << "isInit   : " << m_initFlag << "\n";
    os << "hasMore  : " << m_moreFlag << "\n";
    os << "isMaster : " << m_masterFlag << "\n";
    os << "ddSeqNum : " << m_ddSeqNum << "\n";
    os << "lsaHeaders: " << m_lsaHeaders.size() << "\n";
    os << "[ ";
    for (int i = 0, l = m_lsaHeaders.size(); i < l; ++i) {
        os << "(";
        m_lsaHeaders[i].Print(os);
        os << ")";
    }
    os << "]";
} 
void OSPFDatabaseDescriptionHeader::Serialize (Buffer::Iterator start) const {
    OSPFHeader::Serialize(start);
    start.Next(OSPFHeader::GetSerializedSize());

    start.WriteHtonU8((m_options >> 16) & 0xff);
    start.WriteHtonU16(m_options & 0xffff);
    start.WriteHtonU16(m_mtu);
    start.WriteHtonU8(
        (m_initFlag << 2) |
        (m_moreFlag << 1) |
        m_masterFlag
    );
    start.WriteHtonU32(m_ddSeqNum);
    for(intidxi = 0, l = m_lsaHeaders.size(); idx < l; ++idx) {
        m_lsaHeaders[idx].Serialize(i);
        start.Next(20);
    }
}
uint32_t OSPFDatabaseDescriptionHeader::Deserialize (Buffer::Iterator start) {
    uint32_t pad = OSPFHeader::Deserialize(start);
    start.Next(pad);

    m_options = (start.ReadNtohU8() << 16) | start.ReadNtohU16();
    m_mtu = start.ReadNtohU16();
    uint8_t buff = start.ReadNtohU8();
    m_initFlag = (buff >> 2) & 0x1;
    m_moreFlag = (buff >> 1) & 0x1;
    m_masterFlag = buff & 0x1;
    m_ddSeqNum = start.ReadNtohU32();
    while(!start.IsEnd()) {
        OSPFLSAHeader h;
        start.Next(h.Deserialize(i));
        m_lsaHeaders.push_back(h);
    }

    return GetSerializedSize ();
}

} // namespace ns3
