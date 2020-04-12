//
// Created by zhoutt96 on 4/6/20.
//


#include "utils.h"
#include "RoutingProtocolImpl.h"
#include "utils.h"
#include <unordered_map>
#include <vector>
using namespace std;

void RoutingProtocolImpl::init_DV_alarm(){
    void* ptr = malloc(sizeof(alarmType));
    *((alarmType*)ptr) = DV_ALARM;
    this->sys->set_alarm(this, 30*1000, ptr);
}

void RoutingProtocolImpl::updateDV(unsigned short port, vector<pair<unsigned short, unsigned short>> updateMsg){
    bool isUpdate = false;
    printf("[Update] DV table \n");
    unsigned short sendNode = port;


    DVTable jumpNode = dv_cost_map[sendNode];
    unsigned short jumpCost = jumpNode.cost;
    for(auto a:updateMsg){

        if(a.first == router_id)
            continue;

        if(dv_cost_map.count(a.first) == 0){
            DVTable newRec(a.first,sendNode,jumpCost+a.second);
            dv_cost_map.insert(make_pair(sendNode,newRec));
            forwarding_table.insert(make_pair(a.first,sendNode));
            isUpdate = true;
        }else{
            DVTable neighbor = dv_cost_map[a.first];
            unsigned short newCost = jumpCost+a.second;
            if(newCost<neighbor.cost){
                neighbor.nextHop = port;
                neighbor.cost = newCost;
                dv_cost_map[a.first] = neighbor;
                forwarding_table[a.first] = sendNode;
                isUpdate = true;
            }
        }

    }
    if(isUpdate){
        DV_sendUpdateMsg();
    }

}

void RoutingProtocolImpl::updateDV(unsigned short nei_id, unsigned short costChange){
    bool isUpdate = false;
    for(auto a : dv_cost_map){
        DVTable neighbor = a.second;
        if(neighbor.nextHop == nei_id){
            neighbor.cost += costChange;
            dv_cost_map[a.first] = neighbor;
            isUpdate = true;
        }
    }
    if(isUpdate){
        DV_sendUpdateMsg();
    }
}

void RoutingProtocolImpl::init_DV_Protocol(){
//    printf("Init DV now \n");
    for(auto a:port_map){
        PORT port = a.second;
        DVTable tempRec(port.nei_id,port.nei_id,port.link_cost);
        dv_cost_map.insert(make_pair(port.nei_id,tempRec));
    }

}

void RoutingProtocolImpl::DV_message_handler(unsigned short port, void *packet,unsigned short size){
    printf("[RECV] DV Message \n");

    vector<pair<unsigned short, unsigned short>> updateMsg;
    int i = 8;
    unsigned short nodeID;
    unsigned short nodeCost;
    while(i<size){
        nodeID = ntohs(*(unsigned short *)((char*)packet + i));
        nodeCost = ntohs(*(unsigned short *)((char*)packet + i + 2));
        updateMsg.push_back(make_pair(nodeID,nodeCost));
        i = i + 4;
    }
    updateDV(port,updateMsg);


}

void RoutingProtocolImpl::forward_message_DV(unsigned short port, void *packet, unsigned short size) {
    printf("[Forward] DATA \n");
}

void RoutingProtocolImpl::DV_sendUpdateMsg(){

}