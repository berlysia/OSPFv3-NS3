#ifndef OSPF_DD_TAG_H
#define OSPF_DD_TAG_H

#include "ns3/tag.h"
#include "ospf-lsa-header.h"
#include <vector>

using namespace ns3;

namespace ns3 {

class OSPFDatabaseDescriptionTag : public Tag {
private:
    uint24_t m_options;
    uint16_t m_mtu;
    bool m_initFlag;
    bool m_moreFlag;
    bool m_masterFlag;
    uint32_t m_ddSeqNum;
    vector<OSPFLSAHeader> m_lsaHeaders;

public:
    OSPFDatabaseDescriptionTag () {};

    static TypeId GetTypeId();

    // from `Tag`
    virtual void Deserialize (TagBuffer i); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (TagBuffer i) const;

    void SetOptions(uint24_t options) {m_options = options;}
    uint24_t GetOptions() {return m_options;}
    void SetMtu(uint16_t mtu) {m_mtu = mtu;}
    uint16_t GetMtu() {return m_mtu;}
    void SetInitFlag(bool init) {m_initFlag = init;}
    bool GetInitFlag() {return m_initFlag;}
    void SetMoreFlag(bool more) {m_moreFlag = more;}
    bool GetMoreFlag() {return m_moreFlag;}
    void SetMasterFlag(bool master) {m_masterFlag = master;}
    bool GetMasterFlag() {return m_masterFlag;}
    void SetSequenceNumber(uint32_t ddSeqNum) {m_ddSeqNum = ddSeqNum;}
    uint32_t GetSequenceNumber() {return m_ddSeqNum;}
    void SetLSAHeaders(vector<OSPFLSAHeader> &headers) {m_lsaHeaders = headers;}
    vector<OSPFLSAHeader>& GetLSAHeaders() {return m_lsaHeaders;}
};

}
#endif
