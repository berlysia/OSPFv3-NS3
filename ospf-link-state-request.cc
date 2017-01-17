#include "ospf-link-state-request.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE("OSPFLinkStateRequest");
NS_OBJECT_ENSURE_REGISTERED(OSPFLinkStateRequest);

TypeId OSPFLinkStateRequest::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFLinkStateRequest")
        .SetParent<OSPFHeader>()
        .AddConstructor<OSPFLinkStateRequest>();
    return tid;
}

TypeId OSPFLinkStateRequest::GetInstanceTypeId () const {
    return GetTypeId();
}

/*
vector<uint16_t> m_lsTypes;
vector<uint32_t> m_lsIds;
vector<uint32_t> m_advRtrs;
*/

uint32_t OSPFLinkStateRequest::GetSerializedSize () const {
    return OSPFHeader::GetSerializedSize() + m_ids.size() * 12;
} 
void OSPFLinkStateRequest::Print (std::ostream &os) const {
    os << "(";
    OSPFHeader::Print(os);
    os << "Link State Request - ";
    uint32_t size = m_ids.size();
    os << "identifiers: " << size << "( ";
    for (int i = 0, l = size; i < l; ++i) {
        const OSPFLinkStateIdentifier &identifier = m_ids[i];
        os << "[";
        os << "type: " << identifier.m_type << ", ";
        os << "id: " << identifier.m_id << ", ";
        os << "advRtr: " << identifier.m_advRtr << " ], ";
    }
    os << ")";
} 
void OSPFLinkStateRequest::Serialize (Buffer::Iterator i) const {
/*

      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |              0                |        LS Type                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         Link State ID                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Advertising Router                      |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                 ...                           |
*/
    OSPFHeader::Serialize(i);
    i.Next(OSPFHeader::GetSerializedSize());

    for(int idx = 0, l = m_ids.size(); idx < l; ++idx) {
        const OSPFLinkStateIdentifier &identifier = m_ids[idx];
        i.WriteU16(0);
        i.WriteU16(identifier.m_type);
        i.WriteU32(identifier.m_id);
        i.WriteU32(identifier.m_advRtr);
    }
}
uint32_t OSPFLinkStateRequest::Deserialize (Buffer::Iterator i) {
    i.Next(OSPFHeader::Deserialize(i));

    uint32_t size = (m_packetLength - OSPFHeader::GetSerializedSize()) / 12;
    for(int idx = 0, l = size; idx < l; ++idx) {
        i.ReadU16();
        SetLinkStateIdentifier(i.ReadU16(), i.ReadU32(), i.ReadU32());
    }

    return OSPFLinkStateRequest::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
