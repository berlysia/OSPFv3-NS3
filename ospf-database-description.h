#ifndef OSPF_DD_H
#define OSPF_DD_H

#include "ospf-header.h"
#include "ospf-lsa-header.h"
#include <vector>

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFDatabaseDescription : public OSPFHeader {
private:
    uint32_t m_options;
    uint16_t m_mtu;
    bool m_initFlag;
    bool m_moreFlag;
    bool m_masterFlag;
    uint32_t m_ddSeqNum;
    std::vector<OSPFLSAHeader> m_lsaHeaders;

public:
    OSPFDatabaseDescription () : OSPFHeader () {
        SetType(OSPF_TYPE_DATABASE_DESCRIPTION);
    };
    ~OSPFDatabaseDescription () {};

    static TypeId GetTypeId();

    // from `Header`
    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual uint32_t GetSerializedSize () const;
    virtual void Print (std::ostream &os) const;
    virtual void Serialize (Buffer::Iterator start) const;

    void SetOptions(uint32_t options) {m_options = options;}
    uint32_t GetOptions() const {return m_options;}
    void SetMtu(uint16_t mtu) {m_mtu = mtu;}
    uint16_t GetMtu() const {return m_mtu;}
    void SetInitFlag(bool init) {m_initFlag = init;}
    bool GetInitFlag() const {return m_initFlag;}
    void SetMoreFlag(bool more) {m_moreFlag = more;}
    bool GetMoreFlag() const {return m_moreFlag;}
    void SetMasterFlag(bool master) {m_masterFlag = master;}
    bool GetMasterFlag() const {return m_masterFlag;}
    void SetSequenceNumber(uint32_t ddSeqNum) {m_ddSeqNum = ddSeqNum;}
    uint32_t GetSequenceNumber() const {return m_ddSeqNum;}
    void SetLSAHeaders(std::vector<OSPFLSAHeader> headers) {m_lsaHeaders = headers;}
    std::vector<OSPFLSAHeader>& GetLSAHeaders() {return m_lsaHeaders;}
    bool operator== (const OSPFDatabaseDescription &other) const {
        OSPFHeader sup = *this;
        OSPFHeader oth = other;
        
        return (
            sup == oth &&
            m_options == other.m_options &&
            m_mtu == other.m_mtu &&
            m_initFlag == other.m_initFlag &&
            m_moreFlag == other.m_moreFlag &&
            m_masterFlag == other.m_masterFlag &&
            m_ddSeqNum == other.m_ddSeqNum &&
            m_lsaHeaders == other.m_lsaHeaders
        );
    }
};

}
}
#endif
