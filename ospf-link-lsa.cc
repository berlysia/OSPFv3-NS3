#include "ospf-link-lsa.h"
#include "ns3/log.h"

namespace ns3 {

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
    static TypeId tid = TypeId("ns3::OSPFLinkLSABody")
        .AddConstructor<OSPFLinkLSABody>();
    return tid;
}

uint32_t OSPFLinkLSABody::GetSerializedSize () const {
    uint32_t size = 2 + 2 + 16 + 4;
    for(const Ipv6Prefix& prefix : m_addressPrefixes) {
        uint8_t length = prefix.GetPrefixLength();
        size += 2 + ((length + 31) / 32) * 4;
    }
    return size;
} 
void OSPFLinkLSABody::Print (std::ostream &os) const {
    os << "rtrPriority: " << m_rtrPriority << "\n";
    os << "options: " << m_options << "\n";
    os << "localAddress: " << m_addr << "\n";
    os << "#prefixes   : " << m_addressPrefixes.size() "\n";
    for (int idx = 0, l = m_addressPrefixes.size(); idx < l; ++idx) {
        os << "  prefixOption : " << m_prefixOptions.get(idx) << "\n";
        os << "  prefixLength : " << m_prefixLengthes.get(idx) << "\n";
        os << "  prefix       : " << m_addressPrefixes.get(idx) << "\n";
        os << " ---- " << "\n";
    }
} 
void OSPFLinkLSABody::Serialize (TagBuffer &i) const {
    i.WriteU16(m_rtrPriority);
    i.WriteU16(m_options);
    uint32_t size = m_addressPrefixes.size();
    i.WriteU32(size);
    for (int idx = 0, l = size; idx < l; ++idx) {
        i.WriteU8(m_prefixOptions.get(idx));
        Ipv6Address& addr = m_addressPrefixes.get(idx);
        uint8_t length = m_prefixLengthes.get(idx);
        i.WriteU8(length);
        uint8_t[16] bytes = prefix.GetBytes();
        uint8_t bufSize = (length + 31) / 32 * 4;
        for (int j = 0, ll = (length + 31) / 32 * 4; j < ll; ++j) {
            i.WriteU8(bytes[j]);
        }
    }
}
uint32_t OSPFLinkLSABody::Deserialize (TagBuffer &i) {
    m_rtrPriority = i.ReadU16();
    m_options = i.ReadU16();
    uint32_t size = i.ReadU32();
    m_addressPrefixes.resize(size);
    m_prefixOptions.resize(size);
    m_prefixLengthes.resize(size);
    for (int idx = 0, l = size; idx < l; ++idx) {
        m_prefixOptions[idx] = i.ReadU8();
        m_prefixLengthes[idx] = i.ReadU32();
        uint8_t[16] bytes = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        for (int j = 0, l = (m_prefixLengthes[idx] + 31) / 32 * 4; j < ll; ++j) {
            bytes[j] = i.ReadU8();
        }
        m_addressPrefixes[idx].Set(bytes);
    }
    return GetSerializedSize();
}

} // namespace ns3
