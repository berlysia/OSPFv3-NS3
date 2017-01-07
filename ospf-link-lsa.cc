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
    uint32_t size = 2 + 2 + 16 + 4;
    // for(int idx = 0, l = m_addressPrefixes.size(); idx < l; ++idx) {
    //     uint8_t length = GetPrefixLength(idx);
    //     size += 2 + ((length + 31) / 32) * 4;
    // }
    return size + m_addressPrefixes.size() * 18;
} 
void OSPFLinkLSABody::Print (std::ostream &os) const {
    os << "    ### Link LSA\n";
    os << "      rtrPriority : " << m_rtrPriority << "\n";
    os << "      options     : " << m_options << "\n";
    os << "      localAddress: " << m_addr << "\n";
    os << "      #prefixes   : " << m_addressPrefixes.size() << "\n";
    for (int idx = 0, l = m_addressPrefixes.size(); idx < l; ++idx) {
        os << "        prefixOption : " << m_prefixOptions[idx] << "\n";
        os << "        prefixLength : " << m_prefixLengthes[idx] << "\n";
        os << "        prefix       : " << m_addressPrefixes[idx] << "\n";
    }
} 
void OSPFLinkLSABody::Serialize (Buffer::Iterator &i) const {
    i.WriteHtonU16(m_rtrPriority);
    i.WriteHtonU16(m_options);

    uint8_t buf[16];
    m_addr.GetBytes(buf);
    for (int idx = 0, l = 16; idx < l; ++idx) {
        i.WriteU8(buf[idx]);
    }

    uint32_t size = m_addressPrefixes.size();
    i.WriteHtonU32(size);
    for (int idx = 0, l = size; idx < l; ++idx) {
        i.WriteU8(m_prefixOptions[idx]);
        const Ipv6Address& addr = m_addressPrefixes[idx];
        uint8_t length = m_prefixLengthes[idx];
        i.WriteU8(length);

        addr.GetBytes(buf);

        // uint8_t bufSize = (length + 31) / 32 * 4;
        // for (int j = 0, ll = (length + 31) / 32 * 4; j < ll; ++j) {
        //     i.WriteU8(buf[j]);
        // }

        for (int j = 0, l = 16; j < l; ++j) {
            i.WriteU8(buf[j]);
        }
    }
}
uint32_t OSPFLinkLSABody::Deserialize (Buffer::Iterator &i) {
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
        m_prefixOptions[idx] = i.ReadU8();
        m_prefixLengthes[idx] = i.ReadU8();
        // memset(buf, 0x00, 16);
        // for (int j = 0, ll = (m_prefixLengthes[idx] + 31) / 32 * 4; j < ll; ++j) {
        //     buf[j] = i.ReadU8();
        // }
        for (int j = 0, l = 16; j < l; ++j) {
            buf[j] = i.ReadU8();
        }
        m_addressPrefixes[idx].Set(buf);
    }
    return OSPFLinkLSABody::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
