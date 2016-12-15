#ifndef OSPF_LSA_HEADER_H
#define OSPF_LSA_HEADER_H

#include "ns3/tag.h"
#include "ns3/ipv6-address.h"

using namespace ns3;

namespace ns3 {

class OSPFLSAHeader {
private:
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

    virtual uint32_t Deserialize (TagBuffer i);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (TagBuffer i) const;

    void SetAge(uint16_t age) {m_age = age;}
    uint16_t GetAge() {return m_age;}
    void SetType(uint16_t type) {m_type = type;}
    uint16_t GetType() {return m_type;}
    void SetId(uint32_t id) {m_id = id;}
    uint32_t GetId() {return m_id;}
    void SetAdvertisingRouter(uint32_t advRtr) {m_advRtr = advRtr;}
    uint32_t GetAdvertisingRouter() {return m_advRtr;}
    void SetSequenceNumber(uint32_t seqNum) {m_seqNum = seqNum;}
    uint32_t GetSequenceNumber() {return m_seqNum;}
    void SetChecksum(uint16_t checksum) {m_checksum = checksum;}
    uint16_t GetCheckSum() {return m_checksum;}
    void SetLength(uint16_t length) {m_length = length;}
    uint16_T GetLength() {return m_length;}
};

}
#endif
