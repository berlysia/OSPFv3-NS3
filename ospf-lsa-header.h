#ifndef OSPF_LSA_HEADER_H
#define OSPF_LSA_HEADER_H

#include "ns3/ipv6-address.h"
#include "ns3/object.h"
#include "ns3/buffer.h"

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
    uint16_t m_age;
    uint16_t m_type;
    uint32_t m_id;
    uint32_t m_advRtr;
    uint32_t m_seqNum;
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
    ~OSPFLSAHeader () {};

    static TypeId GetTypeId();

    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t Deserialize (Buffer::Iterator &i);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator &i) const;

    virtual void SetAge(uint16_t age) {m_age = age;}
    virtual uint16_t GetAge() {return m_age;}
    virtual void SetType(uint16_t type) {m_type = type;}
    virtual uint16_t GetType() {return m_type;}
    virtual void SetId(uint32_t id) {m_id = id;}
    virtual uint32_t GetId() {return m_id;}
    virtual void SetAdvertisingRouter(uint32_t advRtr) {m_advRtr = advRtr;}
    virtual uint32_t GetAdvertisingRouter() {return m_advRtr;}
    virtual void SetSequenceNumber(uint32_t seqNum) {m_seqNum = seqNum;}
    virtual uint32_t GetSequenceNumber() {return m_seqNum;}
    virtual void SetChecksum(uint16_t checksum) {m_checksum = checksum;}
    virtual uint16_t GetCheckSum() {return m_checksum;}
    virtual void SetLength(uint16_t length) {m_length = length;}
    virtual uint16_t GetLength() {return m_length;}
    bool operator== (const OSPFLSAHeader &other) const {
        return (
            m_age == other.m_age &&
            m_type == other.m_type &&
            m_id == other.m_id &&
            m_advRtr == other.m_advRtr &&
            m_seqNum == other.m_seqNum &&
            m_checksum == other.m_checksum &&
            m_length == other.m_length
        );
    }
};

}
}
#endif
