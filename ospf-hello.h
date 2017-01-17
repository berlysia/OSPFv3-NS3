#ifndef OSPF_HELLO_H
#define OSPF_HELLO_H

#include "ospf-header.h"

#include <vector>

using namespace ns3;

namespace ns3 {
namespace ospf {

class OSPFHello : public OSPFHeader {
private:
    typedef uint32_t RouterId;
    uint32_t m_interfaceId;
    uint8_t m_routerPriority;
    uint32_t m_options;
    uint16_t m_helloInterval;
    uint16_t m_routerDeadInterval;
    RouterId m_designatedRouterId; // 0ならDRでない
    RouterId m_backupDesignatedRouterId; // 0ならBDRでない
    std::vector<RouterId> m_neighborId;

public:
    OSPFHello () : OSPFHeader () {
        SetType(OSPF_TYPE_HELLO);
    };
    ~OSPFHello () {};

    static TypeId GetTypeId();

    // from `Header`
    virtual TypeId GetInstanceId () const;
    virtual uint32_t Deserialize (Buffer::Iterator start); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator start) const;

    void SetInterfaceId(uint32_t interfaceId) {m_interfaceId = interfaceId;}
    uint32_t GetInterfaceId() const {return m_interfaceId;}
    void SetRouterPriority(uint8_t routerPriority) {m_routerPriority = routerPriority;}
    uint8_t GetRouterPriority() {return m_routerPriority;}
    void SetOptions(uint32_t options) {m_options = options;}
    uint32_t GetOptions() {return m_options;}
    void SetHelloInterval(uint16_t helloInterval) {m_helloInterval = helloInterval;}
    uint16_t GetHelloInterval() {return m_helloInterval;}
    void SetRouterDeadInterval(uint16_t routerDeadInterval) {m_routerDeadInterval = routerDeadInterval;}
    uint16_t GetRouterDeadInterval() {return m_routerDeadInterval;}
    void SetDesignatedRouter(RouterId designatedRouter) {m_designatedRouterId = designatedRouter;};
    RouterId GetDesignatedRouter() {return m_designatedRouterId;}
    void SetBackupDesignatedRouter(RouterId backupDesignatedRouter) {m_backupDesignatedRouterId = backupDesignatedRouter;};
    RouterId GetBackupDesignatedRouter() {return m_backupDesignatedRouterId;}
    void SetNeighbors(std::vector<RouterId> neighbors) {m_neighborId = neighbors;}
    std::vector<RouterId>& GetNeighbors() {return m_neighborId;}
    bool operator== (const OSPFHello &other) const {
        OSPFHeader sup = *this;
        OSPFHeader oth = other;
        
        return (
            sup == oth &&
            m_interfaceId == other.m_interfaceId &&
            m_routerPriority == other.m_routerPriority &&
            m_options == other.m_options &&
            m_helloInterval == other.m_helloInterval &&
            m_routerDeadInterval == other.m_routerDeadInterval &&
            m_designatedRouterId == other.m_designatedRouterId &&
            m_backupDesignatedRouterId == other.m_backupDesignatedRouterId &&
            m_neighborId == other.m_neighborId
        );
    }
};

}
}
#endif
