#ifndef OSPF_LSR_H
#define OSPF_LSR_H

#include "ospf-header.h"
#include <vector>

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
    bool operator== (const OSPFLinkStateIdentifier &other) const {
        return (
            m_type == other.m_type &&
            m_id == other.m_id &&
            m_advRtr == other.m_advRtr
        );
    }
};

class OSPFLinkStateRequest : public OSPFHeader {
private:
    std::vector<OSPFLinkStateIdentifier> m_ids;

public:
    OSPFLinkStateRequest () : OSPFHeader () {
        SetType(OSPF_TYPE_LINK_STATE_REQUEST);
    };
    ~OSPFLinkStateRequest () {};

    static TypeId GetTypeId();

    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t Deserialize (Buffer::Iterator start); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator start) const;

    virtual uint32_t CountLinkStateIdentifiers () {
        return m_ids.size();
    }

    virtual void SetLinkStateIdentifier(uint16_t type, uint32_t id, uint32_t advRtr) {
        m_ids.push_back(OSPFLinkStateIdentifier(type, id, advRtr));
    }
    virtual OSPFLinkStateIdentifier GetLinkStateIdentifier (int index) {
        return m_ids[index];
    }
    bool operator== (const OSPFLinkStateRequest &other) const {
        OSPFHeader sup = *this;
        OSPFHeader oth = other;
        
        return (
            sup == oth &&
            m_ids == other.m_ids
        );
    }
};

}
}
#endif
