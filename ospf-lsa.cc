#include "ospf-lsa.h"
#include "ns3/log.h"

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE("OSPFLSA");
NS_OBJECT_ENSURE_REGISTERED(OSPFLSA);

TypeId OSPFLSA::GetTypeId () {
    static TypeId tid = TypeId("ns3::ospf::OSPFLSA")
        .AddConstructor<OSPFLSA>();
    return tid;
}

uint32_t OSPFLSA::GetSerializedSize () const {
    return (
        (m_header ? m_header->GetSerializedSize() : 0) +
        (m_body ? m_body->GetSerializedSize() : 0)
    );
} 
void OSPFLSA::Print (std::ostream &os) const {
    if (m_header) m_header->Print(os);
    if (m_body) m_body->Print(os);
} 
void OSPFLSA::Serialize (Buffer::Iterator &i) const {
    if (m_header) {
        m_header->Serialize(i);
        // i.Next(m_header->GetSerializedSize());
    }

    if (m_body) {
        m_body->Serialize(i);
        // i.Next(m_body->GetSerializedSize());
    }
}
uint32_t OSPFLSA::Deserialize (Buffer::Iterator &i) {
    if (!m_header) {
        m_header = Create<OSPFLSAHeader>();
    }
    // i.Next(m_header->Deserialize(i));
    m_header->Deserialize(i);

    uint16_t type = m_header->GetType();
    CreateBody(type);

    // i.Next(m_body->Deserialize(i));
    m_body->Deserialize(i);

    return OSPFLSA::GetSerializedSize();
}

} // namespace ns3
} // namespace ns3
