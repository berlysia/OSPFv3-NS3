#ifndef OSPF_LS_UPDATE_H
#define OSPF_LS_UPDATE_H

#include "ospf-header.h"
#include "ospf-lsa.h"
#include <vector>

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFLinkStateUpdate : public OSPFHeader {
private:
    std::vector<Ptr<OSPFLSA> > m_lsas;

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

    virtual uint32_t CountLSAs () const {
        return m_lsas.size();
    }

    void AddLSA(Ptr<OSPFLSA> lsa) {
        m_lsas.push_back(lsa);
    }
    void SetLSAs(std::vector<Ptr<OSPFLSA> >& lsas) {
        m_lsas = lsas;
    }
    Ptr<OSPFLSA> GetLSA (int index) {
        return m_lsas[index];
    }
    std::vector<Ptr<OSPFLSA> >& GetLSAs () {
        return m_lsas;
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
