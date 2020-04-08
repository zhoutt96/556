//
// Created by Zequn Jiang on 3/30/20.
//

#ifndef PROJECT3_DETECTNEIGHBOR_CC
#define PROJECT3_DETECTNEIGHBOR_CC

#include "common.h"
#include "RoutingProtocolImpl.h"

int PingPong_size = 12;

void RoutingProtocolImpl :: createPingPongMessage() {
    unsigned short size = 8 + 4; // header + timestamp
    for (unsigned int i = 0; i < num_ports; i++) {
        char* Ping_pkt = (char *)malloc(sizeof(char) * size);
        *Ping_pkt = (unsigned char)PING;
        *(unsigned short *)(Ping_pkt + 2) = htons(size); // size
        *(unsigned short *)(Ping_pkt + 4) = htons(router_id); // source
        *(unsigned short *)(Ping_pkt + 6) = htons(ports[i].to); // target
        *(unsigned int *)(Ping_pkt + 8) = htonl(sys->time());
        sys->send(i, Ping_pkt, PingPong_size);
    }
}

void RoutingProtocolImpl :: handleMessage(unsigned short port, void *packet, unsigned short size) {
    char *data = (char *)packet;
    ePacketType type = getPacketType(packet);
    if (type == PING) {
        *data = (unsigned char)PONG;
        unsigned short target_id = *(unsigned short *)(data + 4);
        *(unsigned short *)(data + 4) = htons(router_id);
        *(unsigned short *)(data + 6) = htons(target_id);
        *(unsigned int *)(data + 8) = htonl(sys->time());
        sys->send(port, data, PingPong_size);
    }
    else if (type == PONG) {
        unsigned int cur_time = sys->time();
        unsigned int timestamp = ntohl(*(unsigned int *)(data + 8));
        unsigned short RTT = cur_time - timestamp;
        unsigned short neighbor_id = ntohs(*(unsigned short *)(data + 4));
        // update ports table
        ports[port].to = neighbor_id;
        ports[port].cost = RTT;
        ports[port].last_update_time = cur_time;
        bool is_connect = ports[port].is_connect;
        ports[port].is_connect = true;
        if (protocol_type == P_DV) {
            // update direct neighbors table
            if (neighbors.count(neighbor_id) && is_connect) { // already connected before, just update cost
                neighbors[neighbor_id].port = port;
                unsigned short old_RTT =  neighbors[neighbor_id].cost;
                neighbors[neighbor_id].cost = RTT;
                unsigned short RTT_diff = RTT - old_RTT;
                if (RTT_diff != 0) { // cost changes
                    for (auto &entry : *DVM.DV_table) {
                        if (entry.second.next_hop == neighbor_id) { // this neighbor is a next_hop of some destinations
                            unsigned int new_RTT = entry.second.cost + RTT_diff;
                            if (neighbors.count(entry.first) && neighbors[entry.first].cost < new_RTT) { // destination is also a neighbor
                                entry.second.next_hop = entry.first; // directly go to this destination/neighbor
                                entry.second.cost = neighbors[entry.first].cost;
                                forwarding_table[entry.first] = entry.first;
                            }
                            else { // otherwise, use current route and just update cost
                                entry.second.cost += RTT_diff; // may not best route anymore if cost increases
                            }
                            entry.second.last_update_time = sys->time();
                        }
                        else if (entry.first == neighbor_id && RTT < (*DVM.DV_table)[neighbor_id].cost) { // directly go to this neighbor
                            entry.second.next_hop = neighbor_id; // next-hop is itself
                            entry.second.cost = RTT;
                            forwarding_table[neighbor_id] = neighbor_id;
                            entry.second.last_update_time = sys->time();
                        }
                    }
                    DVM.sendUpdatePacket();
                }
                else {
                    for (auto &entry : *DVM.DV_table) {
                        if (entry.second.next_hop == neighbor_id || entry.first == neighbor_id) {
                            entry.second.last_update_time = sys->time();
                        }
                    }
                }
            }
            else { // find a new neighbor or re-connect
                neighbors[neighbor_id] = { port, RTT };
                if (DVM.DV_table->count(neighbor_id) && !is_connect) { // re-connect a neighbor, may change best route for itself
                    if (RTT < (*DVM.DV_table)[neighbor_id].cost) { // if direct route is better
                        DVM.updateDVEntry(neighbor_id, RTT, neighbor_id);
                        DVM.sendUpdatePacket();
                    }
                    else {
                        (*DVM.DV_table)[neighbor_id].last_update_time = sys->time();
                    }
                }
                else { // new neighbor that first time appears
                    DVM.buildDVEntry(neighbor_id, RTT, neighbor_id);
                    DVM.sendUpdatePacket();
                }
                forwarding_table[neighbor_id] = neighbor_id;
            }
        }
        else if (protocol_type == P_LS) {
            if (neighbors.count(neighbor_id)) {
                neighbors[neighbor_id].port = port;
                unsigned short old_RTT = neighbors[neighbor_id].cost;
                neighbors[neighbor_id].cost = RTT;
                if (RTT != old_RTT) { // local changes
                    unsigned int new_time = sys->time();
                    (*LSM.LS_table)[router_id][neighbor_id] = { RTT, new_time };
                    (*LSM.LS_table)[neighbor_id][router_id] = { RTT, new_time };
                    LSM.Dijkstra();
                    LSM.floodLSP();
                }
            }
            else {
                neighbors[neighbor_id] = { port, RTT };
                if (LSM.LS_table->count(neighbor_id)) { // re-connect
                    unsigned int new_time = sys->time();
                    (*LSM.LS_table)[router_id][neighbor_id] = { RTT, new_time };
                    (*LSM.LS_table)[neighbor_id][router_id] = { RTT, new_time };
                }
                else { // first time meet a new neighbor
                    LSM.insertNeighborToGraph(neighbor_id);
                }
                LSM.Dijkstra();
                LSM.floodLSP();
            }
        }
    }
}




#endif //PROJECT3_DETECTNEIGHBOR_CC
