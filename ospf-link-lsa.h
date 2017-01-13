#ifndef OSPF_LINK_LSA_H
#define OSPF_LINK_LSA_H

/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           LS Age              |0|0|0|          8              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Link State ID                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                     Advertising Router                        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                     LS Sequence Number                        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |        LS Checksum            |            Length             |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | Rtr Priority  |                Options                        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      +-                                                             -+
      |                                                               |
      +-                Link-local Interface Address                 -+
      |                                                               |
      +-                                                             -+
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         # prefixes                            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  PrefixLength | PrefixOptions |             0                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        Address Prefix                         |
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  PrefixLength | PrefixOptions |             0                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        Address Prefix                         |
      |                             ...                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                              Link-LSA Format

*/

#include <vector>
#include "ospf-lsa-body.h"
#include "ns3/ipv6-address.h"

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFLinkLSABody : public OSPFLSABody {
private:
    uint16_t m_rtrPriority;
    uint16_t m_options;
    Ipv6Address m_addr;
    std::vector<uint8_t> m_prefixOptions;
    std::vector<uint8_t> m_prefixLengthes;
    std::vector<Ipv6Address> m_addressPrefixes;

public:
    OSPFLinkLSABody () : OSPFLSABody () {};
    ~OSPFLinkLSABody () {};

    static TypeId GetTypeId();

    virtual TypeId GetInstanceId () const;
    virtual uint32_t Deserialize (Buffer::Iterator &i);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator &i) const;

    virtual uint16_t GetRtrPriority() const {return m_rtrPriority;}
    virtual void SetRtrPriority(uint16_t prio) {m_rtrPriority = prio;}
    virtual uint16_t GetOptions() const {return m_options;}
    virtual void SetOptions(uint16_t opt) {m_options = opt;}
    virtual const Ipv6Address& GetLinkLocalAddress() const {return m_addr;}
    virtual void SetLinkLocalAddress(Ipv6Address &addr) {m_addr = addr;}
    virtual uint8_t GetPrefixOption(uint32_t idx) const {return m_prefixOptions[idx];}
    virtual uint8_t GetPrefixLength(uint32_t idx) const {return m_prefixLengthes[idx];}
    virtual const Ipv6Address& GetPrefixAddress(uint32_t idx) const {return m_addressPrefixes[idx];}
    virtual void AddPrefix(Ipv6Address addr, uint8_t prefixLength, uint32_t option = 0) {
        m_prefixLengthes.push_back(prefixLength);
        Ipv6Prefix prefix(prefixLength);
        m_addressPrefixes.push_back(addr.CombinePrefix(prefix));
        m_prefixOptions.push_back(option);
    }
    virtual uint32_t CountPrefixes() const {
        return m_addressPrefixes.size();
    }
    virtual bool operator== (const OSPFLinkLSABody &other) const {
        return (
            m_rtrPriority == other.m_rtrPriority &&
            m_options == other.m_options &&
            m_addr == other.m_addr &&
            m_prefixOptions == other.m_prefixOptions &&
            m_prefixLengthes == other.m_prefixLengthes &&
            m_addressPrefixes == other.m_addressPrefixes
        );
    }
};

}
}
#endif


