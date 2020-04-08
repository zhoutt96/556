//
// Created by Zequn Jiang on 4/3/20.
//

#ifndef PROJECT3_LSMANAGER_H
#define PROJECT3_LSMANAGER_H


#include "Node.h"
#include "common.h"
#include <set>
#include <queue>

class LSManager {
public:
    Node *sys; // To store Node object; used to access GSR9999 interfaces
    unsigned short router_id;
    unsigned short num_ports;
    unordered_map<unsigned short, Neighbor> *neighbors;
    unordered_map<unsigned short, Port> *ports;
    unordered_map<unsigned short, unordered_map<unsigned short, LS_Entry>> *LS_table;
    vector<unsigned short> destinations;
    unordered_map<unsigned short, unsigned int> *seq_map;
    unordered_map<unsigned short, unsigned short> *forwarding_table;
    unsigned int seq_num = 0;
    unsigned short num_nodes = 0;

    void init(Node *sys, unsigned short routerId, unsigned short numPorts,
              unordered_map<unsigned short, Neighbor> *neighbors, unordered_map<unsigned short, Port> *ports,
              unordered_map<unsigned short, unsigned short> *forwardingTable) {
        this->sys = sys;
        this->router_id = routerId;
        this->num_ports = numPorts;
        this->neighbors = neighbors;
        this->ports = ports;
        this->forwarding_table = forwardingTable;

        (*LS_table)[router_id] = { { router_id, {0, sys->time() } } };
    }

    void floodLSP() {
        unsigned short size = neighbors->size() * 4 + 12; // in bytes
        for (unsigned short i = 0; i < num_ports; i++) {
            if ((*ports)[i].is_connect) {
                char *msg = (char *)malloc(size * sizeof(char));
                *msg = (unsigned char)LS;
                unsigned short *packet = (unsigned short *)msg;
                *(packet + 1) = htons(size);
                *(packet + 2) = htons(router_id);
                *(unsigned int *)(packet + 4) = htonl(seq_num);
                seq_num++;
                int count = 0;
                for (auto it : *neighbors) { // send all neighbors
                    unsigned short neighbor_id = it.first;
                    unsigned short cost = it.second.cost;
                    *(packet + 6 + count) = htons(neighbor_id);
                    count++;
                    *(packet + 6 + count) = htons(cost);
                    count++;
                }
                sys->send(i, msg, size);
            }
        }
    }

    void recvLSP(void *data, unsigned int size) {
        unsigned short *pkt = (unsigned short *)data;
        unsigned short source_id = ntohs(*(pkt + 2));
        unsigned int seq = ntohl(*(unsigned int *)(pkt + 4));
        if (seq_map->count(source_id) && (*seq_map)[source_id] <= seq) {
            return ;
        }
        else { // not in map or seq is larger
            (*seq_map)[source_id] = seq;
            vector<pair<unsigned short, unsigned short>> path_pairs;
            int count = 0;
            unsigned int num_pairs = (size - 12) / 4;
            for (unsigned int i = 0; i < num_pairs; i++) {
                unsigned short neighbor_id = ntohs(*(pkt + count + 6));
                count++;
                unsigned short cost = ntohs(*(pkt + count + 6));
                count++;
                path_pairs.push_back({ neighbor_id, cost });
            }
            updateGraph(source_id, path_pairs);
        }
        for (auto &port : *ports) { // flood received LSP again
            if (!port.second.is_connect || port.second.to == source_id) {
                continue;
            }
            else {
                sys->send(port.first, data, size);
            }
        }
    }

    unordered_map<unsigned short, LS_Entry> insertNodeHelper(unsigned short new_dest) {
        num_nodes++;
        unordered_map<unsigned short, LS_Entry> cur_nodes;
        for (auto it : destinations) {
            unsigned int cur_time = sys->time();
            cur_nodes[it] = { INFINITY_COST, cur_time };
            (*LS_table)[it][new_dest] = { INFINITY_COST, cur_time };
        }
        cur_nodes[new_dest] = { 0, sys->time() };
        destinations.push_back(new_dest);
        return cur_nodes;
    }

    void insertNeighborToGraph(unsigned short neighbor_id) {
        LS_table->insert({ neighbor_id, insertNodeHelper(neighbor_id) });
    }

    void updateGraph(unsigned short source_id, const vector<pair<unsigned short, unsigned short>> &path_pairs) {
        if (!LS_table->count(source_id)) {
            LS_table->insert({ source_id, insertNodeHelper(source_id) }); // insert a new node
        }
        for (const auto &it : path_pairs) {
            if (!LS_table->count(it.first)) {
                LS_table->insert({ it.first, insertNodeHelper(it.first) });
            }
            unsigned int cur_time = sys->time();
            (*LS_table)[source_id][it.first] = { it.second, cur_time };
            (*LS_table)[it.first][source_id] = { it.second, cur_time };
        }
    }

    void checkTimeoutEntry() {
        bool flag = false;
        for (auto &port : *ports) {
            if (!port.second.is_connect || sys->time() - port.second.last_update_time <= 15 * 1000) {
                continue;
            }
            else {
                port.second.cost = 0;
                port.second.is_connect = false;
                unsigned short lostNeighbor = port.second.to;
                neighbors->erase(lostNeighbor);
                unsigned int cur_time = sys->time();
                (*LS_table)[lostNeighbor][router_id] = { INFINITY_COST, cur_time };
                (*LS_table)[router_id][lostNeighbor] = { INFINITY_COST, cur_time };
                flag = true;
            }
        }
        for (unsigned short i = 0; i < num_nodes; i++) {
            for (unsigned short j = 0; j <= i; j++) {
                if (sys->time() - (*LS_table)[i][j].last_update_time <= 45000) {
                    continue;
                }
                else {
                    (*LS_table)[i][j].last_update_time = sys->time();
                    (*LS_table)[i][j].cost = INFINITY_COST;
                    flag = true;
                }
            }
        }
        if (flag) { // LS local changes
            Dijkstra();
            floodLSP();
        }
    }


    struct cmp {
        bool operator()(pair<unsigned short, unsigned short> a, pair<unsigned short, unsigned short> b) {
            return a.second > b.second;
        }
    };

    void Dijkstra() {
        auto compare = [](int* lhs, int* rhs) {
            return lhs[2] > rhs[2];
        };
        priority_queue<int*, vector<int*>, decltype(compare)> pq(compare);
        unordered_map<unsigned short, unsigned short> map;
        int n = destinations.size();
        int tmp[]{router_id, router_id, 0};
        pq.push(tmp);
        while (map.size() < n && !pq.empty()) {
            unsigned short cur = INFINITY_COST;
            while (!pq.empty()) {
                int* a = pq.top();
                pq.pop();
                if (map.count(a[1]) == 0) {
                    map[a[1]] = a[2];
                    cur = a[1];
                    if (a[0] != router_id) {
                        (*forwarding_table)[a[1]] = (*forwarding_table)[a[0]];
                    }
                    break;
                }
            }
            if (cur == INFINITY_COST) {
                break;
            }
            auto adjs = (*LS_table)[cur];
            for (auto it : adjs) {
                if (it.second.cost == INFINITY_COST) {
                    continue;
                }
                if (it.first != cur && map.count(it.first) == 0) {
                    int *tmp1 {new int[3]{cur, it.first, map[cur] + it.second.cost}};
                    pq.push(tmp1);
                }
            }
        }

    }

};




#endif //PROJECT3_LSMANAGER_H
