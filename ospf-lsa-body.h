#ifndef OSPF_LSA_BODY_H
#define OSPF_LSA_BODY_H

#include "ospf-lsa-header.h"

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFLSABody : public Object {

public:
    OSPFLSABody () {};
    ~OSPFLSABody () {};

    static TypeId GetTypeId() {
        static TypeId tid = TypeId("ns3::ospf::OSPFLSABody")
            .AddConstructor<OSPFLSABody>();
        return tid;
    };

    virtual TypeId GetInstanceId (void) const {return GetTypeId();};
    virtual uint32_t Deserialize (Buffer::Iterator &i) {return 0;};
    virtual uint32_t GetSerializedSize () const {return 0;}; 
    virtual void Print (std::ostream &os) const {}; 
    virtual void Serialize (Buffer::Iterator &i) const {};
    bool operator== (const OSPFLSABody &other) const {
        return true;
    }
};

}
}
#endif
