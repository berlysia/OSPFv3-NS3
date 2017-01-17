#include "ospf-router-lsa.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

/*
uint32_t m_options;
uint8_t m_type;
uint16_t m_metric;
uint32_t m_interfaceId;
uint32_t m_neighborInterfaceId;
uint32_t m_neighborRouterId;
*/

NS_LOG_COMPONENT_DEFINE("OSPFRouterLSABody");
NS_OBJECT_ENSURE_REGISTERED(OSPFRouterLSABody);

TypeId OSPFRouterLSABody::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFRouterLSABody")
        .SetParent<OSPFLSABody>()
        .AddConstructor<OSPFRouterLSABody>();
    return tid;
}

TypeId OSPFRouterLSABody::GetInstanceId () const {
    return GetTypeId();
}

uint32_t OSPFRouterLSABody::GetSerializedSize () const {
    return 4 + 16 * m_types.size();
} 
void OSPFRouterLSABody::Print (std::ostream &os) const {
    os << "(Router LSA: [";
    os << "options: " << m_options << ", ";
    os << "#lsa: " << m_types.size() << "(";
    for (uint32_t idx = 0, l = m_types.size(); idx < l; ++idx) {
        os << "[";
        os << "type: " << (uint16_t)m_types[idx] << ", ";
        os << "metric: " << m_metrics[idx] << ", ";
        os << "interfaceId: " << m_interfaceIds[idx] << ", ";
        os << "neighborInterfaceId: " << m_neighborInterfaceIds[idx] << ", ";
        os << "neighborRouterId: " << m_neighborRouterIds[idx] << "]";
    }
    os << ")])";
} 
void OSPFRouterLSABody::Serialize (Buffer::Iterator &i) const {
/*
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  0  |Nt|x|V|E|B|            Options                            |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type       |       0       |          Metric               |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                      Interface ID                              |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                   Neighbor Interface ID                        |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Neighbor Router ID                          |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                             ...                                |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type       |       0       |          Metric               |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                      Interface ID                              |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                   Neighbor Interface ID                        |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Neighbor Router ID                          |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                             ...                                |
*/
    i.WriteHtonU32(m_options);
    for (int idx = 0, l = m_types.size(); idx < l; ++idx) {
        i.WriteU8(m_types[idx]);
        i.WriteU8(0);
        i.WriteHtonU16(m_metrics[idx]);
        i.WriteHtonU32(m_interfaceIds[idx]);
        i.WriteHtonU32(m_neighborInterfaceIds[idx]);
        i.WriteHtonU32(m_neighborRouterIds[idx]);
    }
}
uint32_t OSPFRouterLSABody::Deserialize (Buffer::Iterator &i, uint32_t remainBytes) {
    m_options = i.ReadNtohU32();

    uint32_t size = (remainBytes - 4) / 16;
    for (int idx = 0, l = size; idx < l; ++idx) {
        m_types.push_back(i.ReadU8());
        i.ReadU8();
        m_metrics.push_back(i.ReadNtohU16());
        m_interfaceIds.push_back(i.ReadNtohU32());
        m_neighborInterfaceIds.push_back(i.ReadNtohU32());
        m_neighborRouterIds.push_back(i.ReadNtohU32());
    }
    return OSPFRouterLSABody::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
