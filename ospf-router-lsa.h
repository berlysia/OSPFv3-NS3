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
namespace ospf {

class OSPFRouterLSABody : public OSPFLSABody {
private:
    uint32_t m_options;
    std::vector<uint8_t> m_types;
    std::vector<uint16_t> m_metrics;
    std::vector<uint32_t> m_interfaceIds;
    std::vector<uint32_t> m_neighborInterfaceIds;
    std::vector<uint32_t> m_neighborRouterIds;

public:
    OSPFRouterLSABody () : OSPFLSABody() {};
    ~OSPFRouterLSABody () {};

    static TypeId GetTypeId();

    virtual TypeId GetInstanceId () const;
    virtual uint32_t Deserialize (Buffer::Iterator &i);
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator &i) const;

    virtual uint32_t GetSize() {return m_types.size();}
    virtual void SetOptions(uint32_t opts) {m_options = opts;}
    virtual uint32_t GetOptions() {return m_options;}
    virtual void AddType(uint8_t type) {m_types.push_back(type);}
    virtual uint8_t GetType(int idx) {return m_types[idx];}
    virtual void AddInterfaceId(uint32_t id) {m_interfaceIds.push_back(id);}
    virtual uint32_t GetInterfaceId(int idx) {return m_interfaceIds[idx];}
    virtual void AddNeighborInterfaceId(uint32_t id) {m_neighborInterfaceIds.push_back(id);}
    virtual uint32_t GetNeighborInterfaceId(int idx) {return m_neighborInterfaceIds[idx];}
    virtual void AddNeighborRouterId(uint32_t id) {m_neighborRouterIds.push_back(id);}
    virtual uint32_t GetNeighborRouterId(int idx) {return m_neighborRouterIds[idx];}
    virtual bool operator== (const OSPFRouterLSABody &other) const {
        return (
            m_options == other.m_options &&
            m_types == other.m_types &&
            m_metrics == other.m_metrics &&
            m_interfaceIds == other.m_interfaceIds &&
            m_neighborInterfaceIds == other.m_neighborInterfaceIds &&
            m_neighborRouterIds == other.m_neighborRouterIds
        );
    }
};

}
}
#endif

