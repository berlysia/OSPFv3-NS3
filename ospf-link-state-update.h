#ifndef OSPF_LSU_H
#define OSPF_LSU_H

#include "ospf-header.h"
#include "ospf-lsa.h"
#include <vector>

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFLinkStateUpdate : public OSPFHeader {
private:
    std::vector<OSPFLSA> m_lsas;

public:
    OSPFLinkStateUpdate () : OSPFHeader () {
        SetType(OSPF_TYPE_LINK_STATE_UPDATE);
    };
    ~OSPFLinkStateUpdate () {};

    static TypeId GetTypeId();

    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t Deserialize (Buffer::Iterator start); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator start) const;

    virtual uint32_t CountLSAHeaders () const {
        return m_lsas.size();
    }

    virtual void SetLSAHeader(OSPFLSA &lsa) {
        m_lsas.push_back(lsa);
    }
    virtual OSPFLSA GetLSAHeader (int index) const {
        return m_lsas[index];
    }
    bool operator== (const OSPFLinkStateUpdate &other) const {
        OSPFHeader sup = *this;
        OSPFHeader oth = other;
        
        return (
            sup == oth &&
            m_lsas == other.m_lsas
        );
    }
};

}
}
#endif
