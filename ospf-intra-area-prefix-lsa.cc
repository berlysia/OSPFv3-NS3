#include "ospf-intra-area-prefix-lsa.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

/*
uint16_t m_prefixes;
uint16_t m_refType;
uint32_t m_refId;
uint32_t m_refAdvRtr;
std::vector<uint8_t> m_prefixOptions;
std::vector<uint8_t> m_prefixLengthes;
std::vector<uint16_t> m_metrics;
std::vector<Ipv6Address> m_addressPrefixes;
*/

NS_LOG_COMPONENT_DEFINE("OSPFIntraAreaPrefixLSABody");
NS_OBJECT_ENSURE_REGISTERED(OSPFIntraAreaPrefixLSABody);

TypeId OSPFIntraAreaPrefixLSABody::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFIntraAreaPrefixLSABody")
        .SetParent<OSPFLSABody>()
        .AddConstructor<OSPFIntraAreaPrefixLSABody>();
    return tid;
}

TypeId OSPFIntraAreaPrefixLSABody::GetInstanceId () const {
    return GetTypeId();
}

uint32_t OSPFIntraAreaPrefixLSABody::GetSerializedSize () const {
    uint32_t size = 12;
    for(int idx = 0, l = m_addressPrefixes.size(); idx < l; ++idx) {
        uint8_t length = GetPrefixLength(idx);
        size += 4 + ((length + 31) / 32) * 4;
    }
    return size;
    // return size + m_addressPrefixes.size() * 20;
} 
void OSPFIntraAreaPrefixLSABody::Print (std::ostream &os) const {
    os << "(Intra area prefix LSA: [";
    os << "refType: " << m_refType << ", ";
    os << "refId: " << m_refId << ", ";
    os << "refAdvRtr: " << m_refAdvRtr << ", ";
    os << "#prefixes: " << m_addressPrefixes.size() << "(";
    for (int idx = 0, l = m_addressPrefixes.size(); idx < l; ++idx) {
        os << "[metric: " << m_metrics[idx] << ", ";
        os << "prefixOption: " << (uint16_t)m_prefixOptions[idx] << ", ";
        os << "prefixLength: " << (uint16_t)m_prefixLengthes[idx] << ", ";
        os << "prefix: " << m_addressPrefixes[idx] << "], ";
    }
    os << ")])";
} 
void OSPFIntraAreaPrefixLSABody::Serialize (Buffer::Iterator &i) const {
/*
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |         # Prefixes            |     Referenced LS Type        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                  Referenced Link State ID                     |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               Referenced Advertising Router                   |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  PrefixLength | PrefixOptions |          Metric               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Address Prefix                          |
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  PrefixLength | PrefixOptions |          Metric               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Address Prefix                          |
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
    uint32_t size = m_addressPrefixes.size();
    i.WriteHtonU16((uint16_t)size);
    i.WriteHtonU16(m_refType);
    i.WriteHtonU32(m_refId);
    i.WriteHtonU32(m_refAdvRtr);

    uint8_t buf[16];
    for (int idx = 0, l = size; idx < l; ++idx) {
        uint8_t length = m_prefixLengthes[idx];
        i.WriteU8(length);
        i.WriteU8(m_prefixOptions[idx]);
        const Ipv6Address& addr = m_addressPrefixes[idx];
        i.WriteHtonU16(m_metrics[idx]);

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
uint32_t OSPFIntraAreaPrefixLSABody::Deserialize (Buffer::Iterator &i, uint32_t remainBytes) {
    uint16_t size = i.ReadNtohU16();
    m_refType = i.ReadNtohU16();
    m_refId = i.ReadNtohU32();
    m_refAdvRtr = i.ReadNtohU32();

    uint8_t buf[16];
    m_addressPrefixes.resize(size);
    m_prefixOptions.resize(size);
    m_prefixLengthes.resize(size);
    m_metrics.resize(size);
    for (int idx = 0, l = size; idx < l; ++idx) {
        m_prefixLengthes[idx] = i.ReadU8();
        m_prefixOptions[idx] = i.ReadU8();
        m_metrics[idx] = i.ReadNtohU16();

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
    return OSPFIntraAreaPrefixLSABody::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
