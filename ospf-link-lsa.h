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

class OSPFLinkLSABody : public OSPFLSABody {
private:
    uint16_t m_rtrPriority;
    uint16_t m_options;
    Ipv6Address m_addr;
    vector<uint8_t> m_prefixOptions;
    vector<uint8_t> m_prefixLengthes;
    vector<Ipv6Address> m_addressPrefixes;

public:
    OSPFLinkLSABody () : OSPFLSABody (0x0008) {};
    ~OSPFLinkLSABody () {};

    static TypeId GetTypeId();

    virtual uint32_t Deserialize (TagBuffer i);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (TagBuffer i) const;

    virtual uint16_t GetRtrPriority() {return m_rtrPriority;}
    virtual void SetRtrPriority(uint16_t prio) {m_rtrPriority = prio;}
    virtual uint16_t GetOptions() {return m_options;}
    virtual void SetOptions(uint16_t opt) {m_options = opt;}
    virtual Ipv6Address& GetLinkLocalAddress() {return m_addr;}
    virtual void SetLinkLocalAddress(Ipv6Address &addr) {m_addr = addr;}
    virtual uint8_t GetPrefixOption(uint32_t idx) {return m_prefixOptions.get(idx);}
    virtual Ipv6Prefix& GetPrefix(uint32_t idx) {return m_addressPrefixes.get(idx);}
    virtual void AddPrefix(Ipv6Address& addr, uint8_t prefixLength, uint32_t option = 0) {
        m_prefixLengthes.push_back(prefixLength);
        Ipv6Prefix prefix(prefixLength);
        m_addressPrefixes.push_back(addr.CombinePrefix(prefix));
        m_prefixOptions.push_back(option);
    }
    virtual uint32_t CountPrefixes() {
        return m_addressPrefixes.size();
    }
};

}
#endif


