#include "ospf-routing-table.h"

#include <random>
#include <numeric>

namespace ns3 {
namespace ospf {

NS_LOG_COMPONENT_DEFINE ("RoutingTable");

NS_OBJECT_ENSURE_REGISTERED (RoutingTable);

static std::random_device rndseed;
static std::mt19937 mt(rndseed());
static std::uniform_real_distribution<> norm01(0.0, 1.0);

RoutingTable::RoutingTable() {}
RoutingTable::~RoutingTable() {
    for (auto& row : m_nextHop_table) {
        for (auto& item : row) item.clear();
        row.clear();
    }
    for (auto& row : m_nextProb_table) {
        for (auto& item : row) item.clear();
        row.clear();
    }
    m_router_tuples.clear();
}
void RoutingTable::Reset () {
    for (auto& row : m_nextHop_table) {
        for (auto& item : row) {
            item.clear();
            item.resize(m_numOfRouters, 0);
        }
    }
    for (auto& row : m_nextProb_table) {
        for (auto& item : row) {
            item.clear();
            item.resize(m_numOfRouters, 0);
        }
    }
    m_router_tuples.clear();
}
void RoutingTable::FinalizeFlow () {
    NS_LOG_FUNCTION(this);

    for (uint32_t i = 0, il = m_numOfRouters; i < il; ++i) {
        for (uint32_t j = 0, jl = m_numOfRouters; j < jl; ++j) {
            std::vector<uint16_t>& flow = m_nextHop_table[i][j];
            float flowSum = std::accumulate(flow.begin(), flow.end(), 0.0f);
            std::vector<float>& prob = m_nextProb_table[i][j];
            for (uint32_t k = 0, kl = m_numOfRouters; k < kl; ++k) {
                prob[k] = flowSum == 0.0f ? 0.0f : flow[k] / flowSum;
            }
            for (uint32_t k = 1, kl = m_numOfRouters; k < kl; ++k) {
                prob[k] += prob[k-1];
            }
        }
    }

    for (auto& row : m_nextHop_table) {
        for (auto& item : row) item.clear();
        row.clear();
    }
}
RouterId RoutingTable::LookupRoute(RouterId srcRtr, RouterId dstRtr) {
    NS_LOG_FUNCTION(this << srcRtr << dstRtr);

    if (!(srcRtr && dstRtr)) return 0;

    std::vector<float>& probs = m_nextProb_table[srcRtr][dstRtr];
    NS_LOG_INFO("src, dst == "<<srcRtr<<", "<<dstRtr<<" に対する確率分布は");
    for (uint32_t k = 1, kl = m_numOfRouters; k < kl; ++k) {
        NS_LOG_INFO("  ["<<k<<"]: " << probs[k]);
    }
    double randVal = norm01(mt);
    NS_LOG_INFO("乱数値: " << randVal);
    auto iter = std::lower_bound(probs.begin(), probs.end(), randVal);
    NS_LOG_INFO("算出されたネクストホップ: " << iter - probs.begin());
    if (iter == probs.end()) return 0;
    return iter - probs.begin();
}
RouterId RoutingTable::GetNearestRouter(Ipv6Address& target, RouterId ignoreId) {
    NS_LOG_FUNCTION(this << target << ignoreId);

    if (target.IsAny()) {
        return 0;
    }

    for (const auto& tup : m_router_tuples) {
        const Ipv6Address& addr = std::get<0>(tup);
        const Ipv6Prefix& prefix = std::get<1>(tup);
        const RouterId id = std::get<2>(tup);

        if (prefix.IsMatch(addr, target)) {
            if (id != ignoreId)
                return std::get<2>(tup);
        }
    }
    return 0;
}

void RoutingTable::AddFlow(RouterId src, RouterId dst, RouterId nextHop, uint16_t flowValue) {
    NS_LOG_FUNCTION(this << src << dst << nextHop << flowValue);
    m_nextHop_table[src][dst][nextHop] += flowValue;
}

void RoutingTable::AddRouter(Ipv6Address& addr, Ipv6Prefix& prefix, RouterId routerId) {
    NS_LOG_FUNCTION(this << addr << prefix << routerId);
    m_router_tuples.push_back(std::make_tuple(addr, prefix, routerId));
}

void RoutingTable::SetRouters(uint32_t routers) {
    m_numOfRouters = routers;
    m_nextHop_table = std::vector<std::vector<std::vector<uint16_t> > >(
        m_numOfRouters,
        std::vector<std::vector<uint16_t> >(
            m_numOfRouters,
            std::vector<uint16_t>(m_numOfRouters, 0)
        )
    );
    m_nextProb_table = std::vector<std::vector<std::vector<float> > >(
        m_numOfRouters,
        std::vector<std::vector<float> >(
            m_numOfRouters,
            std::vector<float>(m_numOfRouters, 0.0f)
        )
    );
    m_router_tuples.reserve(m_numOfRouters);
}

std::ostream& operator<< (std::ostream& os, const RoutingTable& table) {
    for (const auto& tup : table.m_router_tuples) {
        const Ipv6Address& addr = std::get<0>(tup);
        const Ipv6Prefix& prefix = std::get<1>(tup);
        const RouterId id = std::get<2>(tup);
        os << addr << prefix << " - router " << id << "\n";
    }

    float diff = 0.0f;
    for (uint32_t i = 1, il = table.m_numOfRouters; i < il; ++i) {
        for (uint32_t j = 1, jl = table.m_numOfRouters; j < jl; ++j) {
            if (i == j) continue;
            const std::vector<float>& probs = table.m_nextProb_table[i][j];
            os << "[src "<<i<<"][dst "<<j<<"]\n";
            for (uint32_t k = 1, kl = table.m_numOfRouters; k < kl; ++k) {
                diff = probs[k] - probs[k-1];
                if (std::abs(diff) < 1e-6) continue;
                os << "[src "<<i<<"][dst "<<j<<"][next "<<k<<"]: " << probs[k] << "\n";
            }
        }
    }
    return os;
}

}
}