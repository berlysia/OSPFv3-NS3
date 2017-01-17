#ifndef OSPF_INTRA_AREA_PREFIX_LSA_H
#define OSPF_INTRA_AREA_PREFIX_LSA_H

#include <vector>
#include "ospf-lsa-body.h"
#include "ns3/ipv6-address.h"

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFIntraAreaPrefixLSABody : public OSPFLSABody {
private:
    // uint16_t m_prefixes;
    uint16_t m_refType;
    uint32_t m_refId; // router-LSAなら0、network-LSAならDRのrouterId
    uint32_t m_refAdvRtr;
    std::vector<uint8_t> m_prefixOptions;
    std::vector<uint8_t> m_prefixLengthes;
    std::vector<uint16_t> m_metrics;
    std::vector<Ipv6Address> m_addressPrefixes;

public:
    OSPFIntraAreaPrefixLSABody () : OSPFLSABody () {};
    ~OSPFIntraAreaPrefixLSABody () {};

    static TypeId GetTypeId();

    virtual TypeId GetInstanceId () const;
    virtual uint32_t Deserialize (Buffer::Iterator &i, uint32_t remainBytes);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator &i) const;

    virtual uint16_t CountPrefixes() const {return m_addressPrefixes.size();}
    virtual uint16_t GetReferenceType() const {return m_refType;}
    virtual void SetReferenceType(uint16_t type) {m_refType = type;}
    virtual uint32_t GetReferenceLinkStateId () const {return m_refId;}
    virtual void SetReferenceLinkStateId (uint32_t id) {m_refId = id;}
    virtual uint32_t GetReferenceAdvertisedRouter () const {return m_refAdvRtr;}
    virtual void SetReferenceAdvertisedRouter (uint32_t advRtr) {m_refAdvRtr = advRtr;}
    virtual uint8_t GetPrefixOption(uint32_t idx) const {return m_prefixOptions[idx];}
    virtual uint8_t GetPrefixLength(uint32_t idx) const {return m_prefixLengthes[idx];}
    virtual uint16_t GetPrefixMetric(uint32_t idx) const {return m_metrics[idx];}
    virtual const Ipv6Address& GetPrefixAddress(uint32_t idx) const {return m_addressPrefixes[idx];}
    virtual void AddPrefix(Ipv6Address addr, uint8_t prefixLength, uint16_t metric, uint32_t option = 0) {
        m_prefixLengthes.push_back(prefixLength);
        Ipv6Prefix prefix(prefixLength);
        m_addressPrefixes.push_back(addr.CombinePrefix(prefix));
        m_prefixOptions.push_back(option);
        m_metrics.push_back(metric);
    }
    virtual bool operator== (const OSPFIntraAreaPrefixLSABody &other) const {
        return (
            m_refType == other.m_refType &&
            m_refId == other.m_refId &&
            m_refAdvRtr == other.m_refAdvRtr &&
            m_prefixOptions == other.m_prefixOptions &&
            m_prefixLengthes == other.m_prefixLengthes &&
            m_metrics == other.m_metrics &&
            m_addressPrefixes == other.m_addressPrefixes
        );
    }
};

}
}
#endif


