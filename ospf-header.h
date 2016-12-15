#ifndef OSPF_HEADER_H
#define OSPF_HEADER_H

#include "ns3/header.h"
#include "ns3/buffer.h"

#include <vector>

using namespace ns3;

namespace ns3 {
class OSPFHeader : public Header {
private:
    typeset uint32_t RouterId
    uint8_t m_version;
    uint8_t m_type;
    uint16_t m_packetLength;
    RouterId m_routerId;
    uint32_t m_areaId;
    uint16_t m_checksum;
    uint8_t m_instanceId;
    uint8_t m_padding;

public:
    OSPFHeader () {};
    virtual ~OSPFHeader();

    static TypeId GetTypeId();

    // from `Header`
    virtual uint32_t Deserialize (Buffer::Iterator start); 
    virtual uint32_t GetSerializedSize () const; 
    virtual void Print (std::ostream &os) const; 
    virtual void Serialize (Buffer::Iterator start) const;

    void SetVersion(uint8_t version) {m_version = version;}
    uint8_t GetVersion() const {return m_version;}
    void SetType(uint8_t type) {m_type = type;}
    uint8_t GetType() const {return m_type;}
    void SetPacketLength(uint16_t packetLength) {m_packetLength = packetLength;}
    uint16_t GetPacketLength() const {return m_packetLength;}
    void SetAreaId(uint8_t areaId) {m_areaId = areaId;}
    uint8_t GetAreaId() const {return m_areaId;}
    void SetRouterId(RouterId routerId) {m_routerId = routerId;}
    RouterId GetRouterId() const {return m_routerId;}
    void SetAreaId(uint32_t areaId) {m_areaId = areaId;}
    uint32_t GetAreaId() const {return m_areaId;}
    void SetChecksum(uint16_t checksum) {m_checksum = checksum;}
    uint16_t GetChecksum() const {return m_checksum;}
    void SetInstanceId(uint8_t instanceId) {m_instanceId = instanceId;}
    uint8_t GetInstanceId() const {return m_instanceId;}
};

}
#endif
