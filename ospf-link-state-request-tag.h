#ifndef OSPF_LSR_TAG_H
#define OSPF_LSR_TAG_H

#include "ns3/tag.h"
#include <vector>

using namespace ns3;

namespace ns3 {

class OSPFLinkStateRequestTag : public Tag {
private:
    vector<uint16_t> m_lsTypes;
    vector<uint32_t> m_lsIds;
    vector<uint32_t> m_advRtrs;

public:
    OSPFLinkStateRequestTag () {};

    static TypeId GetTypeId();

    // from `Tag`
    virtual void Deserialize (TagBuffer i); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (TagBuffer i) const;

    void AddRequest(uint16_t type, uint32_t id, uint32_t advRtr) {
        m_lsTypes.push_back(type);
        m_lsIds.push_back(id);
        m_advRtrs.push_back(advRtr);
    }
};

}
#endif
