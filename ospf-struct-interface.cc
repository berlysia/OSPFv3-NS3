#include <iostream>
#include "ospf-struct-interface.h"

namespace ns3 {
namespace ospf {

std::string ToString (const InterfaceType& type ) {
    switch(type) {
        case InterfaceType::P2P: return "InterfaceType::P2P";
        case InterfaceType::BROADCAST: return "InterfaceType::BROADCAST";
        case InterfaceType::NBMA: return "InterfaceType::NBMA";
        case InterfaceType::P2M: return "InterfaceType::P2M";
        case InterfaceType::VIRTUAL: return "InterfaceType::VIRTUAL";
    }
    return "InterfaceType::UNKNOWN";
}

std::string ToString (const InterfaceState& value ) {
    switch(value) {
        case InterfaceState::DOWN: return "InterfaceState::DOWN";
        case InterfaceState::P2P: return "InterfaceState::P2P";
        case InterfaceState::LOOPBACK: return "InterfaceState::LOOPBACK";
        case InterfaceState::WAITING: return "InterfaceState::WAITING";
        case InterfaceState::DR_OTHER: return "InterfaceState::DR_OTHER";
        case InterfaceState::BACKUP: return "InterfaceState::BACKUP";
        case InterfaceState::DR: return "InterfaceState::DR";
    }
    return "InterfaceState::UNKNOWN";
}

std::string ToString (const InterfaceEvent& value ) {
    switch(value) {
        case InterfaceEvent::IF_UP: return "InterfaceEvent::IF_UP";
        case InterfaceEvent::WAIT_TIMER: return "InterfaceEvent::WAIT_TIMER";
        case InterfaceEvent::BACKUP_SEEN: return "InterfaceEvent::BACKUP_SEEN";
        case InterfaceEvent::NEIGH_CHANGE: return "InterfaceEvent::NEIGH_CHANGE";
        case InterfaceEvent::LOOP_IND: return "InterfaceEvent::LOOP_IND";
        case InterfaceEvent::UNLOOP_IND: return "InterfaceEvent::UNLOOP_IND";
        case InterfaceEvent::IF_DOWN: return "InterfaceEvent::IF_DOWN";
    }
    return "InterfaceEvent::UNKNOWN";
}

}
}
