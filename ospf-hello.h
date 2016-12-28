#ifndef OSPF_HELLO_H
#define OSPF_HELLO_H

#include "ospf-header.h"

#include <vector>

using namespace ns3;

namespace ns3 {

class OSPFHello : public OSPFHeader {
private:
    typeset uint32_t RouterId
    uint8_t m_routerPriority;
    uint24_t m_options;
    uint16_t m_helloInterval;
    uint16_t m_routerDeadInterval;
    RouterId m_designatedRouterId; // 0ならDRでない
    RouterId m_backupDesinatedRouterId; // 0ならBDRでない
    vector<RouterId> m_neighborId;

public:
    OSPFHelloTag () {};

    static TypeId GetTypeId();

    // from `Header`
    virtual uint32_t Deserialize (Buffer::Iterator start); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator start) const;

    void SetRouterPriority(uint8_t routerPriority) {m_routerPriority = routerPriority;}
    uint8_t GetRouterPriority() {return m_routerPriority;}
    void SetOptions(uint24_t options) {m_options = options;}
    uint24_t GetOptions() {return m_options;}
    void SetHelloInterval(uint16_t helloInterval) {m_helloInterval = helloInterval;}
    uint16_t GetHelloInterval() {return m_helloInterval;}
    void SetRouterDeadInterval(uint16_t routerDeadInterval) {m_routerDeadInterval = routerDeadInterval;}
    uint16_t GetRouterDeadInterval() {return m_routerDeadInterval;}
    void SetDesignatedRouter(RouterId designatedRouter) {m_designatedRouter = designatedRouter;};
    RouterId GetDesignatedRouter() {return m_designatedRouter;}
    void SetBackupDesignatedRouter(RouterId backupDesignatedRouter) {m_backupDesignatedRouter = backupDesignatedRouter;};
    RouterId GetBackupDesignatedRouter() {return m_backupDesinatedRouter;}
    void SetNeighbors(vector<RouterId> &neighbors) {m_neighbors = neighbors;}
    vector<RouterId>& GetNeighbors() {return m_neighbors;}
};

}
#endif
