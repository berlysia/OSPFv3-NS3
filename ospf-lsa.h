#ifndef OSPF_LSA_H
#define OSPF_LSA_H

#include "ospf-lsa-header.h"
#include "ospf-lsa-body.h"

#include "ospf-router-lsa.h"
#include "ospf-link-lsa.h"
#include "ospf-intra-area-prefix-lsa.h"

#include <iostream>

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFLSA : public Object {
private:
    Ptr<OSPFLSAHeader> m_header;
    Ptr<OSPFLSABody> m_body;

public:
    OSPFLSA () {
        // std::cout << "OSPFLSA::ctor - " << this << std::endl;
    };
    OSPFLSA (const OSPFLSA& other) {
        // std::cout << "OSPFLSA::copy - " << this << " <- " << &other << std::endl;
        m_header = other.m_header;
        m_body = other.m_body;
    }
    ~OSPFLSA () {
        // std::cout << "\nOSPFLSA::dtor - " << this << " : " << GetIdentifier() << "( " << m_header << ", " << m_body << " )" << std::endl;
    };

    static TypeId GetTypeId();

    virtual TypeId GetInstanceId (void) const {return GetTypeId();};
    virtual uint32_t Deserialize (Buffer::Iterator &i);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator &i) const;

    void Initialize (uint16_t type) {
        CreateHeader(type);
        CreateBody(type);
    }

    void CreateHeader (uint16_t type) {
        m_header = Create<OSPFLSAHeader>();
        m_header->SetType(type);
    }

    void CreateBody (uint16_t type) {
        switch(type) {
            case OSPF_LSA_TYPE_ROUTER: {
                m_body = Create<OSPFRouterLSABody>();
            } break;
            case OSPF_LSA_TYPE_LINK: {
                m_body = Create<OSPFLinkLSABody>();
            } break;
            case OSPF_LSA_TYPE_INTRA_AREA_PREFIX: {
                m_body = Create<OSPFIntraAreaPrefixLSABody>();
            } break;
        }
    }

    OSPFLinkStateIdentifier GetIdentifier () {
        return GetHeader()->CreateIdentifier();
    }

    bool IsDeprecatedInstance() {
        return GetHeader()->IsDeprecatedInstance();
    }

    Ptr<OSPFLSAHeader> GetHeader () {return m_header;}
    Ptr<OSPFLSABody> GetBody () {return m_body;}
    template <typename T> Ptr<T> GetBody () {
        return DynamicCast<T>(m_body);
    }
    virtual bool operator== (const OSPFLSA &other) const {
        if (m_header && m_body) {
            return (
                *m_header == *other.m_header &&
                *m_body == *other.m_body
            );
        }
        if (!m_header && m_body) {
            return *m_body == *other.m_body;
        }
        if (!m_body && m_header) {
            return *m_header == *other.m_header;
        }
        return (
            m_header == other.m_header &&
            m_body == other.m_body
        );
    }
    virtual bool operator== (const OSPFLinkStateIdentifier &other) const {
        if (m_header) {
            return *m_header == other;
        }
        return false;
    }
};
std::ostream& operator<< (std::ostream& os, const OSPFLSA& lsa);
std::ostream& operator<< (std::ostream& os, std::vector<Ptr<OSPFLSA> >& lsas);

}
}
#endif
