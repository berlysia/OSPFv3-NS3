#ifndef OSPF_LSA_H
#define OSPF_LSA_H

#include "ospf-header.h"
#include "ospf-lsa-header.h"
#include <vector>

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFLinkStateAck : public OSPFHeader {
private:
    std::vector<OSPFLSAHeader> m_lsaHeaders;

public:
    OSPFLinkStateAck () : OSPFHeader () {
        SetType(OSPF_TYPE_LINK_STATE_ACK);
    };
    ~OSPFLinkStateAck () {};

    static TypeId GetTypeId();

    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t Deserialize (Buffer::Iterator start); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator start) const;

    void SetLSAHeaders(std::vector<OSPFLSAHeader> headers) {m_lsaHeaders = headers;}
    std::vector<OSPFLSAHeader>& GetLSAHeaders() {return m_lsaHeaders;}
    bool operator== (const OSPFLinkStateAck &other) const {
        OSPFHeader sup = *this;
        OSPFHeader oth = other;
        
        return (
            sup == oth &&
            m_lsaHeaders == other.m_lsaHeaders
        );
    }
};

}
}
#endif
