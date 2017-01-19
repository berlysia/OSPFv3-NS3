#include <vector>
#include <utility> // pair
#include <unordered_map>
#include <stack>
#include <queue>
#include <iostream>
#include <algorithm>

using namespace std;

typedef uint32_t RouterId;
typedef unordered_map<RouterId, uint16_t> NextHopEntries;
typedef vector<vector<NextHopEntries> > RoutingTable;

vector<vector<uint16_t> > capacityTable, flowTable; // [from][to] = capacity
vector<vector<RouterId> > adjacentTable; // [from][idx] = to;
RoutingTable routingTable; // [m_routerId][from][to][next] =  flow;

int main () {
    int32_t routers, connections;
    RouterId m_routerId;
    cin >> m_routerId;
    cin >> routers >> connections;

    capacityTable.resize(routers, vector<uint16_t>(routers, 0));
    flowTable.resize(routers, vector<uint16_t>(routers, 0));
    adjacentTable.resize(routers);
    routingTable.resize(routers, vector<NextHopEntries>(routers));

    {
        uint32_t from, to, cap;
        for (int32_t i = 0; i < connections; ++i) {
            cin >> from >> to >> cap;
            capacityTable[from][to] = cap;
            adjacentTable[from].push_back(to);
        }
    }

    queue<RouterId> que;
    stack<RouterId> sta;
    vector<int32_t> level(routers);
    vector<uint32_t> iter(routers);
    RouterId curr, fr, to;
    uint32_t routeFlow = 65535;
    for (int32_t src = 0, cl = routers; src < cl; ++src) {
        for (int32_t dst = 0, rl = routers; dst < rl; ++dst) {
            if (src == dst) continue;

            for (auto& row: flowTable) fill(row.begin(), row.end(), 0);

            cout << "iterate: " << src << " -> " << dst << " 経路を探します" << endl;

            for (;;) {
                // 階層算出
                cout << "( " << src << ", " << dst << " )" << "levelを計算します" << endl;
                que.push(src);
                level.assign(routers, -1);
                level[src] = 0;
                while (!que.empty()) {
                    fr = que.front(); que.pop();
                    for (uint32_t idx = 0, adjs = adjacentTable[fr].size(); idx < adjs; ++idx) {
                        to = adjacentTable[fr][idx];
                        if ((capacityTable[fr][to] - flowTable[fr][to]) > 0 && level[to] < 0) {
                            level[to] = level[fr] + 1;
                            que.push(to);
                        }
                    }
                }

                cout << "( " << src << ", " << dst << " )" << "levelを表示します" << endl;
                for (int i = 0, l = routers; i < l; ++i) {
                    cout << "  " << i << ": " << level[i] << endl;
                }

                if (level[dst] == -1) {
                    cout << "iterate end: level["<<dst<<"] == -1" << endl;
                    break; // END
                }

                if (iter[src] == routers) {
                    cout << "iterate end: iter["<<src<<"] == " << routers << endl;
                    break; // END
                }
                
                // 増加道算出
                cout << "( " << src << ", " << dst << " )" << "aug-pathを計算します" << endl;

                routeFlow = 65535;
                iter.assign(routers, 0);
                sta.push(src);
                while (!sta.empty()) {
                    curr = sta.top();
                    cout << "curr: " << curr << ", iter: " << iter[curr] << ", dst: " << dst << endl;
                    if (curr == dst) {
                        // 経路をたどってMTU算出
                        vector<RouterId> routeFrom, routeTo;
                        RouterId nextHop = 4294967295;
                        cout << "GOAL! - stack size: " << sta.size() << endl;
                        while (sta.size() >= 2) {
                            to = sta.top(); sta.pop();
                            fr = sta.top();
                            routeFrom.push_back(fr);
                            routeTo.push_back(to);
                            if (fr == m_routerId) {
                                nextHop = to;
                            }
                            routeFlow = min(routeFlow, (uint32_t)(capacityTable[fr][to] - flowTable[fr][to]));
                        }
                        while (!sta.empty()) sta.pop();

                        if (nextHop != 4294967295) {
                            routingTable[src][dst][nextHop] = routeFlow;
                        }
                        for (int i = 0, l = routeFrom.size(); i < l; ++i) {
                            // routingTables[routeFrom[i]][src][dst][routeTo[i]] = routeFlow;
                            flowTable[routeFrom[i]][routeTo[i]] += routeFlow;
                            flowTable[routeTo[i]][routeFrom[i]] -= routeFlow;
                        }

                        break;
                    }

                    for (RouterId& next = iter[curr]; next < routers; ++next) {
                        if (capacityTable[curr][next] == 0) continue;
                        cout << "  curr: " << curr << ", next: " << next << endl;
                        cout << "  level["<<curr<<"] < level["<<next<<"] = " << level[curr] << " < " << level[next] << "("<<boolalpha<<(level[curr] < level[next]);
                        cout << "), caps["<<curr<<"]["<<next<<"]: " << (capacityTable[curr][next] - flowTable[curr][next]) << " => " << (level[curr] < level[next] && (capacityTable[curr][next] - flowTable[curr][next]) > 0) << endl;
                        if (level[curr] < level[next] && (capacityTable[curr][next] - flowTable[curr][next]) > 0) {
                            sta.push(next);
                            ++next;
                            break;
                        }
                    }
                    if (iter[sta.top()] == routers) {
                        cout << "poped " << sta.top() << ": iter["<<sta.top()<<"]: == "<<routers<<" => " << (iter[curr] == routers) << endl;
                        sta.pop();
                    }
                }
            }
        }
    }

    // while (cin >> m_routerId) {
        for (uint32_t s = 0, sl = routers; s < sl; ++s) {
            for (uint32_t d = 0, dl = routers; d < dl; ++d) {
                if (s == d) continue;
                cout << "table for (" << s << ", " << d << ")\n";
                for (auto& kv : routingTable[s][d]) {
                    cout << "  " << kv.first << ": " << kv.second << endl;
                }
            }
        }       
    // }

    return 0;
}

