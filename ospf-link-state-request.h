#ifndef OSPF_LSR_H
#define OSPF_LSR_H

#include "ns3/buffer.h"
#include "ns3/header.h"
#include <vector>

using namespace ns3;

namespace ns3 {

class OSPFLinkStateIdentifier {
public:
    uint16_t m_type;
    uint32_t m_id;
    uint32_t m_advRtr;
    OSPFLinkStateIdentifier
    (
        uint16_t type,
        uint32_t id,
        uint32_t advRtr
    ) : 
        m_type(type),
        m_id(id),
        m_advRtr(advRtr) {}
    ~OSPFLinkStateIdentifier () {}
};

class OSPFLinkStateRequest : public Tag {
private:
    vector<uint16_t> m_lsTypes;
    vector<uint32_t> m_lsIds;
    vector<uint32_t> m_advRtrs;

public:
    OSPFLinkStateRequest () {};

    static TypeId GetTypeId();

    // from `Tag`
    virtual void Deserialize (TagBuffer i); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (TagBuffer i) const;

    virtual uint32_t CountLinkStateIdentifiers () {
        return m_lsTypes.size();
    }

    virtual void GetLinkStateIdentifier(uint16_t type, uint32_t id, uint32_t advRtr) {
        m_lsTypes.push_back(type);
        m_lsIds.push_back(id);
        m_advRtrs.push_back(advRtr);
    }
    virtual OSPFLinkStateIdentifier& GetLinkStateIdentifier (int index) {
        OSPFLinkStateIdentifier identifier(
            m_lsTypes[index],
            m_lsIdx[index],
            m_advRtrs[index]
        );

        return identifier;
    }
};

}
#endif
