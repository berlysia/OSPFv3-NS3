#include "ospf-router-lsa.h"
#include "ns3/log.h"

namespace ns3 {

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
    static TypeId tid = TypeId("ns3::OSPFRouterLSABody")
        .AddConstructor<OSPFRouterLSABody>();
    return tid;
}

uint32_t OSPFRouterLSABody::GetSerializedSize () const {
    return 4 + 15 * m_types.size();
} 
void OSPFRouterLSABody::Print (std::ostream &os) const {
    os << "options: " << m_options << "\n";
    os << "#lsa   : " << m_types.size() "\n";
    for (int idx = 0, l = m_types.size(); idx < l; ++idx) {
        os << "  type               : " << m_types.get(idx) << "\n";
        os << "  metric             : " << m_metrics.get(idx) << "\n";
        os << "  interfaceId        : " << m_interfaceIds.get(idx) << "\n";
        os << "  neighborInterfaceId: " << m_neighborInterfaceIds.get(idx) << "\n";
        os << "  neighborRouterId   : " << m_neighborRouterIds.get(idx) << "\n";
    }
} 
void OSPFRouterLSABody::Serialize (TagBuffer i) const {
    i.WriteU32(m_options);
    for (int idx = 0, l = m_types.size(); idx < l; ++idx) {
        i.WriteU8(m_types.get(idx));
        i.WriteU16(m_metrics.get(idx));
        i.WriteU32(m_interfaceIds.get(idx));
        i.WriteU32(m_neighborInterfaceIds.get(idx));
        i.WriteU32(m_neighborRouterIds.get(idx));
    }
}
uint32_t OSPFRouterLSABody::Deserialize (TagBuffer i) {
    m_options = i.ReadU32();
    while (!i.IsEnd()) {
        m_types.push_back(i.ReadU8());
        m_metrics.push_back(i.ReadU16());
        m_interfaceIds.push_back(i.ReadU32());
        m_neighborInterfaceIds.push_back(i.ReadU32());
        m_neighborRouterIds.push_back(i.ReadU32());
    }
    return GetSerializedSize();
}

} // namespace ns3
