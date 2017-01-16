#ifndef OSPF_LS_IDENTIFIER_H
#define OSPF_LS_IDENTIFIER_H

#include <set>

using namespace ns3;

namespace ns3 {
namespace ospf {

struct OSPFLinkStateIdentifier {
    uint16_t m_type;
    uint32_t m_id;
    uint32_t m_advRtr;
    OSPFLinkStateIdentifier() {}
    OSPFLinkStateIdentifier(uint16_t type, uint32_t id, uint32_t advRtr)
        : m_type(type), m_id(id), m_advRtr(advRtr) {}
    bool IsOriginatedBy (const uint32_t routerId, std::set<uint32_t>& rtrIfaceId_set) const {
        // 0x2002 is type value for network-LSA
        return m_advRtr == routerId || (m_type == 0x2002 && rtrIfaceId_set.count(m_id));
    }
    OSPFLinkStateIdentifier As(uint16_t type) const {
        return OSPFLinkStateIdentifier(type, m_id, m_advRtr);
    }
    bool operator== (const OSPFLinkStateIdentifier &other) const {
        return (
            m_type == other.m_type &&
            m_id == other.m_id &&
            m_advRtr == other.m_advRtr
        );
    }
    bool operator< (const OSPFLinkStateIdentifier &other) const {
        return (
            m_type < other.m_type ||
            (m_type == other.m_type && m_id < other.m_id) ||
            (m_type == other.m_type && m_id == other.m_id && m_advRtr < other.m_advRtr)
        );
    }
};

}
}

#endif