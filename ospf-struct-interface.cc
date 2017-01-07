#include "ospf-struct-interface.h"

namespace ns3 {
namespace ospf {

InterfaceData::InterfaceData () {
    m_areaId = 0;
    m_helloInterval = Seconds(10.0);
    m_routerDeadInterval = Seconds(10.0);
    m_ifaceTransDelay = 1;
    m_designatedRouterId = 0;
    m_backupDesignatedRouterId = 0;
    m_ifaceOutputCost = 1;
    m_rxmtInterval = Seconds(5.0);
    m_auType = 0;
    m_rxmtTimer.SetFunction(&InterfaceData::RxmtWrapper, this);
}

void InterfaceData::UpdateState(InterfaceState s) {
    m_state = s;
}

void InterfaceData::NotifyEvent(RouterId routerId, RouterId neighborRouterId, NeighborEvent event) {
    // OSPFv2 10.3 The Neighbor state machine
    // https://tools.ietf.org/html/rfc2328#page-89
    NeighborData &neighbor = m_neighbors[neighborRouterId];
    switch (event) {
    case NeighborEvent::START: {
        if (neighbor.IsState(NeighborState::DOWN)) {
            neighbor.SetState(NeighborState::ATTEMPT);
            SendHello();
            neighbor.SetInactivityTimerCallback(&InterfaceData::NotifyEvent, this);
            neighbor.SetInactivityTimerArguments(neighborRouterId, NeighborEvent::INACTIVE);
            neighbor.ResetInactivityTimer(m_routerDeadInterval);
        }
        return;
    }
    case NeighborEvent::HELLORECEIVED: {
        if (neighbor.IsState(NeighborState::DOWN) ||
            neighbor.IsState(NeighborState::ATTEMPT))
        {
            neighbor.SetState(NeighborState::INIT);
        }
        neighbor.SetInactivityTimerCallback(&InterfaceData::NotifyEvent, this);
        neighbor.SetInactivityTimerArguments(neighborRouterId, NeighborEvent::INACTIVE);
        neighbor.ResetInactivityTimer(m_routerDeadInterval);
        return;
    }
    case NeighborEvent::TWOWAYRECEIVED: {
        if (neighbor.IsState(NeighborState::INIT)) {
            if (ShouleBeAdjacent(routerId, neighbor)) {
                neighbor.SetState(NeighborState::EXSTART);
                neighbor.StartExchange();
                SendDatabaseDescription(neighborRouterId);
            } else {
                neighbor.SetState(NeighborState::TWOWAY);
            }
        }
        return;
    }
    case NeighborEvent::NEGOTIATIONDONE: {
        if (neighbor.IsState(NeighborState::EXSTART)) {
            neighbor.SetState(NeighborState::EXCHANGE);
            // DDによるやりとりがはじまる (OSPFv2の10.8と10.6を参照)
        }
        return;
    }
    case NeighborEvent::EXCHANGEDONE: {
        if (neighbor.IsState(NeighborState::EXCHANGE)) {
            if (neighbor.IsRequestListEmpty()) {
                neighbor.SetState(NeighborState::FULL);
            } else {
                neighbor.SetState(NeighborState::LOADING);
                SendLinkStateRequest(neighborRouterId);
            }
        }
        return;
    }
    case NeighborEvent::LOADINGDONE: {
        if (neighbor.IsState(NeighborState::LOADING)) {
            neighbor.SetState(NeighborState::FULL);
        }
        return;
    }
    case NeighborEvent::ISADJOK: {
        if (neighbor.IsState(NeighborState::TWOWAY)) {
            if (ShouleBeAdjacent(routerId, neighbor)) {
                neighbor.SetState(NeighborState::EXSTART);
                neighbor.StartExchange();
                SendDatabaseDescription(neighborRouterId);
            } else {
                // nop
            }
            return;
        }
        if (neighbor.GetState() >= NeighborState::EXSTART) {
            if (ShouleBeAdjacent(routerId, neighbor)) {
                // nop
            } else {
                neighbor.SetState(NeighborState::TWOWAY);
                neighbor.ClearList();
            }
        }
        return;
    }
    case NeighborEvent::BADLSREQ: // fall through
    case NeighborEvent::SEQNUMMISMATCH: {
        if (neighbor.GetState() >= NeighborState::EXCHANGE) {
            neighbor.SetState(NeighborState::EXSTART);
            neighbor.ClearList();
            neighbor.StartExchange();
            SendDatabaseDescription(neighborRouterId);
        }
        return;
    }
    case NeighborEvent::KILLNBR: // fall through
    case NeighborEvent::LLDOWN: // fall through
    case NeighborEvent::INACTIVE: {
        neighbor.SetState(NeighborState::DOWN);
        neighbor.ClearList();
        return;
    }
    case NeighborEvent::ONEWAYRECEIVED: {
        if (neighbor.GetState() >= NeighborState::TWOWAY) {
            neighbor.SetState(NeighborState::INIT);
            neighbor.ClearList();
        }
        return;
    }
    } // switch
}

bool InterfaceData::ShouleBeAdjacent(RouterId routerId, NeighborData &neighbor) {
    // OSPFv2 10.4.  Whether to become adjacent
    // https://tools.ietf.org/html/rfc2328#page-96
    return (
        m_type     == InterfaceType::P2P ||
        m_type     == InterfaceType::P2M ||
        m_type     == InterfaceType::VIRTUAL ||
        routerId   == m_designatedRouterId ||
        routerId   == m_backupDesignatedRouterId ||
        neighbor.m_routerId  == m_designatedRouterId ||
        neighbor.m_routerId  == m_backupDesignatedRouterId
    );
}

void InterfaceData::SendDatabaseDescription(RouterId routerId) {
    if(!m_sendDDCallback.IsNull()){
        m_sendDDCallback(routerId);
    }
}

void InterfaceData::ScheduleSendDatabaseDescription(RouterId routerId) {
    m_rxmtCallback = MakeCallback(&InterfaceData::SendDatabaseDescription, this);
    m_rxmtTimer.SetArguments(routerId);
    SendDatabaseDescription(routerId);
}

void InterfaceData::SendLinkStateRequest(RouterId routerId) {
    if(!m_sendLSRCallback.IsNull()){
        m_sendLSRCallback(routerId);
    }
}

void InterfaceData::ScheduleSendLinkStateRequest(RouterId routerId) {
    m_rxmtCallback = MakeCallback(&InterfaceData::SendLinkStateRequest, this);
    m_rxmtTimer.SetArguments(routerId);
    SendLinkStateRequest(routerId);
}

void InterfaceData::SendHello() {
    if (!m_sendHelloCallback.IsNull()) {
        m_sendHelloCallback();
    }
}

void InterfaceData::ScheduleSendHello() {
    m_helloTimer.SetFunction(&InterfaceData::HelloWrapper, this);
    HelloWrapper();
}

void InterfaceData::RxmtWrapper(RouterId routerId) {
    if (!m_rxmtCallback.IsNull()) {
        m_rxmtCallback(routerId);
    }
    m_rxmtTimer.Schedule(m_rxmtInterval);
}

void InterfaceData::HelloWrapper() {
    SendHello();
    m_helloTimer.Schedule(m_helloInterval);
}

void InterfaceData::CancelRxmt () {
    m_rxmtTimer.Cancel();
}

}
}
