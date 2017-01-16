#include <iostream>

#include "ospf-struct-neighbor.h"

namespace ns3 {
namespace ospf {

std::string ToString ( const NeighborState& value ) {
    switch(value) {
        case NeighborState::DOWN: return "NeighborState::DOWN";
        case NeighborState::ATTEMPT: return "NeighborState::ATTEMPT";
        case NeighborState::INIT: return "NeighborState::INIT";
        case NeighborState::TWOWAY: return "NeighborState::TWOWAY";
        case NeighborState::EXSTART: return "NeighborState::EXSTART";
        case NeighborState::EXCHANGE: return "NeighborState::EXCHANGE";
        case NeighborState::LOADING: return "NeighborState::LOADING";
        case NeighborState::FULL: return "NeighborState::FULL";
    }
    return "NeighborState::UNKNOWN";
}

std::string ToString ( const NeighborEvent& value ) {
    switch(value) {
        case NeighborEvent::HELLO_RECEIVED: return "NeighborEvent::HELLO_RECEIVED";
        case NeighborEvent::START: return "NeighborEvent::START";
        case NeighborEvent::TWOWAY_RECEIVED: return "NeighborEvent::TWOWAY_RECEIVED";
        case NeighborEvent::NEGOT_DONE: return "NeighborEvent::NEGOT_DONE";
        case NeighborEvent::EXCHANGE_DONE: return "NeighborEvent::EXCHANGE_DONE";
        case NeighborEvent::BAD_LS_REQ: return "NeighborEvent::BAD_LS_REQ";
        case NeighborEvent::LOADING_DONE: return "NeighborEvent::LOADING_DONE";
        case NeighborEvent::IS_ADJ_OK: return "NeighborEvent::IS_ADJ_OK";
        case NeighborEvent::SEQ_NUM_MISMATCH: return "NeighborEvent::SEQ_NUM_MISMATCH";
        case NeighborEvent::ONEWAY_RECEIVED: return "NeighborEvent::ONEWAY_RECEIVED";
        case NeighborEvent::KILL_NBR: return "NeighborEvent::KILL_NBR";
        case NeighborEvent::INACTIVE: return "NeighborEvent::INACTIVE";
        case NeighborEvent::LL_DOWN: return "NeighborEvent::LL_DOWN";
    }
    return "NeighborEvent::UNKNOWN";
}

}
}