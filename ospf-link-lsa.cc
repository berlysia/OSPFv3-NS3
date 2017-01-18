#include "ospf-link-lsa.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

/*
uint16_t m_rtrPriority;
uint16_t m_options;
Ipv6Address m_addr;
vector<uint8_t> m_prefixOptions;
vector<Ipv6Prefix> m_addressPrefixes;
*/

NS_LOG_COMPONENT_DEFINE("OSPFLinkLSABody");
NS_OBJECT_ENSURE_REGISTERED(OSPFLinkLSABody);

TypeId OSPFLinkLSABody::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFLinkLSABody")
        .SetParent<OSPFLSABody>()
        .AddConstructor<OSPFLinkLSABody>();
    return tid;
}

TypeId OSPFLinkLSABody::GetInstanceId () const {
    return GetTypeId();
}

uint32_t OSPFLinkLSABody::GetSerializedSize () const {
    uint32_t size = 24;
    for(int idx = 0, l = m_addressPrefixes.size(); idx < l; ++idx) {
        uint8_t length = GetPrefixLength(idx);
        size += 4 + ((length + 31) / 32) * 4;
    }
    return size;
    // return 24 + m_addressPrefixes.size() * 18;
} 
void OSPFLinkLSABody::Print (std::ostream &os) const {
    os << "(Link LSA: [";
    os << "rtrPriority: " << m_rtrPriority << ", ";
    os << "options: " << m_options << ", ";
    os << "localAddress: " << m_addr << ", ";
    os << "#prefixes: " << m_addressPrefixes.size() << "(";
    for (int idx = 0, l = m_addressPrefixes.size(); idx < l; ++idx) {
        os << "[";
        os << "prefixOption: " << (uint16_t)m_prefixOptions[idx] << ", ";
        os << "prefixLength: " << (uint16_t)m_prefixLengthes[idx] << ", ";
        os << "prefix: " << m_addressPrefixes[idx] << "]";
    }
    os << ")])";
} 
void OSPFLinkLSABody::Serialize (Buffer::Iterator &i) const {
/*
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | Rtr Priority  |                Options                        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      +-                                                             -+
      |                                                               |
      +-                Link-local Interface Address                 -+
      |                                                               |
      +-                                                             -+
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         # prefixes                            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  PrefixLength | PrefixOptions |             0                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        Address Prefix                         |
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  PrefixLength | PrefixOptions |             0                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        Address Prefix                         |
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
    i.WriteHtonU32(m_options);
    i.Prev(4);
    i.WriteU8(m_rtrPriority);
    i.Next(3);

    uint8_t buf[16];
    m_addr.GetBytes(buf);
    for (int idx = 0, l = 16; idx < l; ++idx) {
        i.WriteU8(buf[idx]);
    }

    uint32_t size = m_addressPrefixes.size();
    i.WriteHtonU32(size);
    for (int idx = 0, l = size; idx < l; ++idx) {
        uint8_t length = m_prefixLengthes[idx];
        i.WriteU8(length);
        i.WriteU8(m_prefixOptions[idx]);
        i.WriteHtonU16(0);
        const Ipv6Address& addr = m_addressPrefixes[idx];

        addr.GetBytes(buf);

        uint8_t bufSize = (length + 31) / 32 * 4;
        for (int j = 0, ll = bufSize; j < ll; ++j) {
            i.WriteU8(buf[j]);
        }

        // for (int j = 0, l = 16; j < l; ++j) {
        //     i.WriteU8(buf[j]);
        // }
    }
}
uint32_t OSPFLinkLSABody::Deserialize (Buffer::Iterator &i, uint32_t remainBytes) {
    m_rtrPriority = i.ReadNtohU16();
    m_options = i.ReadNtohU16();

    uint8_t buf[16];
    for (int j = 0, l = 16; j < l; ++j) {
        buf[j] = i.ReadU8();
    }
    m_addr.Set(buf);

    uint32_t size = i.ReadNtohU32();
    m_addressPrefixes.resize(size);
    m_prefixOptions.resize(size);
    m_prefixLengthes.resize(size);
    for (int idx = 0, l = size; idx < l; ++idx) {
        m_prefixLengthes[idx] = i.ReadU8();
        m_prefixOptions[idx] = i.ReadU8();
        i.ReadNtohU16();
        memset(buf, 0x00, 16);
        uint8_t bufSize = (m_prefixLengthes[idx] + 31) / 32 * 4;
        for (int j = 0, ll = bufSize; j < ll; ++j) {
            buf[j] = i.ReadU8();
        }
        // for (int j = 0, l = 16; j < l; ++j) {
        //     buf[j] = i.ReadU8();
        // }
        m_addressPrefixes[idx].Set(buf);
    }
    return OSPFLinkLSABody::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
