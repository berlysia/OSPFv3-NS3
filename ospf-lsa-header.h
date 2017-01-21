#ifndef OSPF_LSA_HEADER_H
#define OSPF_LSA_HEADER_H

#include "ns3/ipv6-address.h"
#include "ns3/object.h"
#include "ns3/buffer.h"
#include "ospf-lsa-identifier.h"
#include "ospf-constants.h"
#include <cstdlib>
#include <iostream>

/*

       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           LS Age              |           LS Type             |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Link State ID                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Advertising Router                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    LS Sequence Number                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |        LS Checksum            |             Length            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                              The LSA Header

*/

using namespace ns3;

namespace ns3 {
namespace ospf {

#define OSPF_LSA_TYPE_ROUTER 0x2001
#define OSPF_LSA_TYPE_NETWORK 0x2002
#define OSPF_LSA_TYPE_INTER_AREA_PREFIX 0x2003
#define OSPF_LSA_TYPE_INTER_AREA_ROUTER 0x2004
#define OSPF_LSA_TYPE_AS_EXTERNAL 0x4005
// #define OSPF_LSA_TYPE_DEPRECATED 0x2006
#define OSPF_LSA_TYPE_NSSA 0x2007
#define OSPF_LSA_TYPE_LINK 0x0008
#define OSPF_LSA_TYPE_INTRA_AREA_PREFIX 0x2009

class OSPFLSAHeader : public Object {
protected:
    typedef uint32_t RouterId;
    uint16_t m_age;
    uint16_t m_type;
    uint32_t m_id;
    RouterId m_advRtr;
    int32_t m_seqNum;
    uint16_t m_checksum;
    uint16_t m_length;
    /*
    types
        1: router LSA(from all router)
        2: network LSA(from DR)
        3: network summary LSA(from ABR)
        4: ASBR summary LSA(from ABR)
        5: AS external LSA(from ASBR)
        7: NSSA external LSA(from ASBR in NSSA)
        8: Link LSA(OSPFv3, from all router)
        9: Intra Area Prefix LSA
    */

public:
    OSPFLSAHeader () {};
    OSPFLSAHeader (const OSPFLSAHeader &o) {
        m_age = o.m_age;
        m_type = o.m_type;
        m_id = o.m_id;
        m_advRtr = o.m_advRtr;
        m_seqNum = o.m_seqNum;
        m_checksum = o.m_checksum;
        m_length = o.m_length;
    };
    OSPFLSAHeader &operator= (const OSPFLSAHeader &o) {
        m_age = o.m_age;
        m_type = o.m_type;
        m_id = o.m_id;
        m_advRtr = o.m_advRtr;
        m_seqNum = o.m_seqNum;
        m_checksum = o.m_checksum;
        m_length = o.m_length;
        return (*this);
    };
    ~OSPFLSAHeader () {};

    static TypeId GetTypeId();

    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t Deserialize (Buffer::Iterator &i);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator &i) const;
    virtual void Serialize (Buffer::Iterator &i, uint32_t bodySize) const;

    virtual void SetAge(uint16_t age) {m_age = age;}
    virtual uint16_t GetAge() {return m_age;}
    virtual void SetType(uint16_t type) {m_type = type;}
    virtual uint16_t GetType() {return m_type;}
    virtual bool IsLinkLocalScope () {return (m_type & 0xf000) == 0;}
    virtual bool IsAreaScope () {return (m_type & 0xf000) == 2;}
    virtual bool IsASScope () {return (m_type & 0xf000) == 4;}
    virtual void SetId(uint32_t id) {m_id = id;}
    virtual uint32_t GetId() {return m_id;}
    virtual void SetAdvertisingRouter(RouterId advRtr) {m_advRtr = advRtr;}
    virtual RouterId GetAdvertisingRouter() {return m_advRtr;}
    virtual void SetSequenceNumber(int32_t seqNum) {m_seqNum = seqNum;}
    virtual int32_t GetSequenceNumber() {return m_seqNum;}
    virtual void IncrementSequenceNumber() {m_seqNum++;}
    virtual void InitializeSequenceNumber() {m_seqNum = g_initialSeqNum;}
    virtual void SetChecksum(uint16_t checksum) {m_checksum = checksum;}
    virtual uint16_t GetCheckSum() {return m_checksum;}
    virtual void SetLength(uint16_t length) {m_length = length;}
    virtual uint16_t GetLength() {return m_length;}
    OSPFLinkStateIdentifier CreateIdentifier () const {
        return OSPFLinkStateIdentifier(m_type, m_id, m_advRtr);
    }
    bool operator== (const OSPFLSAHeader &other) const {
        return (
            m_age == other.m_age &&
            m_type == other.m_type &&
            m_id == other.m_id &&
            m_advRtr == other.m_advRtr &&
            m_seqNum == other.m_seqNum
            // m_checksum == other.m_checksum
            // m_length == other.m_length
        );
    }
    bool operator== (const OSPFLinkStateIdentifier &id) const {
        return CreateIdentifier() == id;
    }

    bool IsMoreRecentThan (const OSPFLSAHeader &other) const {
        if (m_seqNum > other.m_seqNum) return true;
        if (m_seqNum < other.m_seqNum) return false;
        // if (m_checksum > other.m_checksum) return true;
        // if (m_checksum < other.m_checksum) return false;
        if (m_age == g_maxAge) return true;
        if (other.m_age == g_maxAge) return false;
        if (std::abs(m_age - other.m_age) > g_maxAgeDiff) {
            if (m_age < other.m_age) return true;
            if (m_age > other.m_age) return false;
        }
        return false;
    }

    bool IsSameIdentity (const OSPFLSAHeader &other) const {
        return (
            m_type == other.m_type &&
            m_id == other.m_id &&
            m_advRtr == other.m_advRtr
        );
    }

    bool IsSameInstance (const OSPFLSAHeader &other) const {
        return (
            m_type == other.m_type &&
            m_id == other.m_id &&
            m_advRtr == other.m_advRtr &&
            m_seqNum == other.m_seqNum &&
            // m_checksum == other.m_checksum &&
            // m_age == other.m_age &&
            m_age != g_maxAge &&
            other.m_age != g_maxAge
        );
    }

    bool IsDeprecatedInstance () {
        return m_age == g_maxAge && m_seqNum == g_maxSeqNum;
    }
};
std::ostream& operator<< (std::ostream& os, const OSPFLSAHeader& lsaHdr);
std::ostream& operator<< (std::ostream& os, std::vector<Ptr<OSPFLSAHeader> >& lsaHdrs);

}
}
#endif
