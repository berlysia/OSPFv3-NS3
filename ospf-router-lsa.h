#ifndef OSPF_ROUTER_LSA_H
#define OSPF_ROUTER_LSA_H

/*
       0                    1                   2                   3
       0 1 2 3  4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           LS Age               |0|0|1|         1               |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Link State ID                            |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Advertising Router                          |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    LS Sequence Number                          |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |        LS Checksum             |            Length             |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  0  |Nt|x|V|E|B|            Options                            |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type       |       0       |          Metric               |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                      Interface ID                              |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                   Neighbor Interface ID                        |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Neighbor Router ID                          |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                             ...                                |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type       |       0       |          Metric               |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                      Interface ID                              |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                   Neighbor Interface ID                        |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Neighbor Router ID                          |
      +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                             ...                                |

                             Router-LSA Format

*/

#include <vector>
#include "ospf-lsa-body.h"

using namespace ns3;

namespace ns3 {

class OSPFRouterLSABody : public OSPFLSABody {
private:
    uint32_t m_options;
    vector<uint8_t> m_types;
    vector<uint16_t> m_metrics;
    vector<uint32_t> m_interfaceIds;
    vector<uint32_t> m_neighborInterfaceIds;
    vector<uint32_t> m_neighborRouterIds;

public:
    OSPFRouterLSABody () : OSPFLSABody(0x2001) {};
    ~OSPFRouterLSABody () {};

    static TypeId GetTypeId();

    virtual uint32_t Deserialize (TagBuffer i);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (TagBuffer i) const;

    virtual uint32_t GetSize() {return m_types.size();}
    virtual void SetOptions(uint32_t opts) {m_options = opts;}
    virtual uint32_t GetOptions() {return m_options;}
    virtual void AddType(uint8_t type) {m_types.push_back(type);}
    virtual uint8_t GetType(int idx) {return m_types.get(idx);}
    virtual void AddInterfaceId(uint32_t id) {m_interfaceIds.push_back(id);}
    virtual uint32_t GetInterfaceId(int idx) {return m_interfaceIds.get(idx);}
    virtual void AddNeighborInterfaceId(uint32_t id) {m_neighborInterfaceIds.push_back(id);}
    virtual uint32_t GetNeighborInterfaceId(int idx) {return m_neighborInterfaceIds.get(idx);}
    virtual void AddNeighborRouterId(uint32_t id) {m_neighborRouterIds.push_back(id);}
    virtual uint32_t GetNeighborRouterId(int idx) {return m_neighborRouterIds.get(idx);}
};

}
#endif

