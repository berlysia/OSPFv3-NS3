#ifndef OSPF_HEADER_H
#define OSPF_HEADER_H

#include "ns3/header.h"
#include "ns3/buffer.h"

#include <vector>

using namespace ns3;

namespace ns3 {
namespace ospf {

#define OSPF_TYPE_HELLO 1
#define OSPF_TYPE_DATABASE_DESCRIPTION 2
#define OSPF_TYPE_LINK_STATE_REQUEST 3
#define OSPF_TYPE_LINK_STATE_UPDATE 4
#define OSPF_TYPE_LINK_STATE_ACK 5

class OSPFHeader : public Header {
protected:
    typedef uint32_t RouterId;
    uint8_t m_version;
    uint8_t m_type;
    uint16_t m_packetLength;
    RouterId m_routerId;
    uint32_t m_areaId;
    uint16_t m_checksum;
    uint8_t m_instanceId;

public:
    OSPFHeader () {
        m_version = 3;
    };
    virtual ~OSPFHeader() {};

    static TypeId GetTypeId();

    // from `Header`
    virtual TypeId GetInstanceTypeId (void) const;
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
    void SetChecksum(uint16_t checksum) {m_checksum = checksum;}
    uint16_t GetChecksum() const {return m_checksum;}
    void SetInstanceId(uint8_t instanceId) {m_instanceId = instanceId;}
    uint8_t GetInstanceId() const {return m_instanceId;}
    bool operator== (const OSPFHeader &other) const {

        // Print(std::cout);
        // other.Print(std::cout);

        // std::cout << std::boolalpha << "OSPFHeader: "
        //     << (m_version == other.m_version) << " && "
        //     << (m_type == other.m_type) << " && "
        //     << (m_packetLength == other.m_packetLength) << " && "
        //     << (m_routerId == other.m_routerId) << " && "
        //     << (m_areaId == other.m_areaId) << " && "
        //     << (m_checksum == other.m_checksum) << " && "
        //     << (m_instanceId == other.m_instanceId) << "\n";
        
        return (
            m_version == other.m_version &&
            m_type == other.m_type &&
            m_packetLength == other.m_packetLength &&
            m_routerId == other.m_routerId &&
            m_areaId == other.m_areaId &&
            m_checksum == other.m_checksum &&
            m_instanceId == other.m_instanceId
        );
    }
};

}
}
#endif
