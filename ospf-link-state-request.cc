#include "ospf-link-state-request.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OSPFLinkStateRequest");
NS_OBJECT_ENSURE_REGISTERED(OSPFLinkStateRequest);

TypeId OSPFLinkStateRequest::GetTypeId () {
    static TypeId tid = TypeId("ns3::OSPFLinkStateRequest")
        .SetParent<Tag>()
        .AddConstructor<OSPFLinkStateRequest>();
    return tid;
}

/*
vector<uint16_t> m_lsTypes;
vector<uint32_t> m_lsIds;
vector<uint32_t> m_advRtrs;
*/

uint32_t OSPFLinkStateRequest::GetSerializedSize () const {
    return (2 + 4 + 4) * m_lsTypes.size();
} 
void OSPFLinkStateRequest::Print (std::ostream &os) const {
    os << "["
    for (int i = 0, l = m_lsTypes.size(); i < l; ++i) {
        os << "  type  : " << m_lsTypes[i] << "\n";
        os << "  id    : " << m_lsIds[i] << "\n";
        os << "  advRtr: " << m_advRtrs[i] << "\n";
    }
    os << "]";
} 
void OSPFLinkStateRequest::Serialize (TagBuffer i) const {
    for(int idx = 0, l = m_lsTypes.size(); idx < l; ++idx) {
        i.WriteU16(m_lsTypes[idx]);
        i.WriteU32(m_lsIds[idx]);
        i.WriteU32(m_advRtrs[idx]);
    }
}
void OSPFLinkStateRequest::Deserialize (TagBuffer i) {
    while(!i.IsEnd()) {
        m_lsTypes.push_back(i.ReadU16());
        m_lsIds.push_back(i.ReadU32());
        m_advRtrs.push_back(i.ReadU32());
    }
}

} // namespace ns3
