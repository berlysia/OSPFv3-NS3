#ifndef OSPF_LS_REQUEST_H
#define OSPF_LS_REQUEST_H

#include "ospf-header.h"
#include "ospf-lsa-identifier.h"
#include <vector>

using namespace ns3;

namespace ns3 {
namespace ospf {

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
    virtual void SetLinkStateIdentifier(OSPFLinkStateIdentifier id) {
        m_ids.push_back(id);
    }
    virtual OSPFLinkStateIdentifier GetLinkStateIdentifier (int index) {
        return m_ids[index];
    }
    virtual void SetLinkStateIdentifiers(std::vector<OSPFLinkStateIdentifier> lsids) {
        m_ids = lsids;
    }
    virtual std::vector<OSPFLinkStateIdentifier> GetLinkStateIdentifiers () {
        return m_ids;
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
