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
vector<Ptr<OSPFLSAHeader> > m_lsaHeaders;
*/

uint32_t OSPFDatabaseDescription::GetSerializedSize () const {
    return OSPFHeader::GetSerializedSize() + 12 + m_lsaHeaders.size() * 20;
} 
void OSPFDatabaseDescription::Print (std::ostream &os) const {
    OSPFHeader::Print(os);
    os << "## DatabaseDescription Packet (";
    os << "options: " << m_options << ", ";
    os << "MTU: " << m_mtu << ", ";
    os << std::boolalpha;
    os << "isInit: " << m_initFlag << ", ";
    os << "hasMore: " << m_moreFlag << ", ";
    os << "isMaster: " << m_masterFlag << ", ";
    os << std::noboolalpha;
    os << "ddSeqNum: " << m_ddSeqNum << ", ";
    os << "lsaHeader: " << m_lsaHeaders.size() << ", ";
    for (int i = 0, l = m_lsaHeaders.size(); i < l; ++i) {
        m_lsaHeaders[i]->Print(os);
    }
} 
void OSPFDatabaseDescription::Serialize (Buffer::Iterator start) const {
/*
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
      |       0       |               Options                          |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
      |        Interface MTU          |      0        |0|0|0|0|0|I|M|MS|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
      |                    DD sequence number                          |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
      |                                                                |
      +-                                                              -+
      |                                                                |
      +-                     An LSA Header                            -+
      |                                                                |
      +-                                                              -+
      |                                                                |
      +-                                                              -+
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
      |                       ...                                      |
*/
    OSPFHeader::Serialize(start);
    start.Next(OSPFHeader::GetSerializedSize());

    start.WriteHtonU32(m_options);
    start.WriteHtonU16(m_mtu);
    start.WriteHtonU16(
        (m_initFlag << 2) |
        (m_moreFlag << 1) |
        m_masterFlag
    );
    start.WriteHtonU32(m_ddSeqNum);
    // uint32_t size = m_lsaHeaders.size();
    // start.WriteHtonU32(size);
    for(int idx = 0, l = m_lsaHeaders.size(); idx < l; ++idx) {
        m_lsaHeaders[idx]->Serialize(start);
    }
}
uint32_t OSPFDatabaseDescription::Deserialize (Buffer::Iterator start) {
    start.Next(OSPFHeader::Deserialize(start));
    m_options = start.ReadNtohU32();
    m_mtu = start.ReadNtohU16();
    uint8_t buff = start.ReadNtohU16();
    m_initFlag = (buff >> 2) & 0x1;
    m_moreFlag = (buff >> 1) & 0x1;
    m_masterFlag = buff & 0x1;
    m_ddSeqNum = start.ReadNtohU32();
    
    uint32_t size = (m_packetLength - GetSerializedSize()) / 20;
    m_lsaHeaders.resize(size);
    for(int idx = 0, l = size; idx < l; ++idx) {
        m_lsaHeaders[idx] = Create<OSPFLSAHeader>();
        m_lsaHeaders[idx]->Deserialize(start);
    }

    return OSPFDatabaseDescription::GetSerializedSize ();
}

} // namespace ns3
} // namespace ns3
