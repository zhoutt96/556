//
// Created by zhoutt96 on 4/6/20.
//


#include "RoutingProtocolImpl.h"

unsigned int RoutingProtocolImpl::ls_seq_num = 0;
void RoutingProtocolImpl::init_LS_alarm(){
//    void* ptr = malloc(sizeof(alarmType));
//    *((alarmType*)ptr) = LS_ALARM;
//    this->sys->set_alarm(this, 35*1000, ptr);
}

void RoutingProtocolImpl::updateLS(){
//    printf("[Update] LS Table \n");
//    this->flooding_lsp();
}

void RoutingProtocolImpl::init_LS_Protocol(){
//    topology_graph =(unsigned short **)malloc(sizeof(unsigned short) * (this->num_of_port));
//    for(int i=0;i<num_of_port;i++)
//        topology_graph[i]=new unsigned short[num_of_port];
//
//    for (int i=0; i<num_of_port;i++){
//        for (int j=0; j<num_of_port;j++){
//            topology_graph[i][j] = 0;
//        }
//    }
//
//    this->init_LS_alarm();
//    this->flooding_lsp();
}

void RoutingProtocolImpl::LS_message_handler(unsigned short port, void *packet,unsigned short size){
//    printf("[RECV] LS Message \n");
//    unsigned int lsp_seq_id =ntohs( *(unsigned int *)((char*)packet + 8));
//    unsigned short source_id = ntohs(*(unsigned short *)((char*)packet + 4));
//    if (this->lsp_seq_set.count(lsp_seq_id)){
//        // receive this seq_id before, discard and do nothing
//        return;
//    }else{
//        // store it into the set, and forward to all neighbors except the source id
//        this->lsp_seq_set.insert(lsp_seq_id);
//        for (auto it=this->port_map.begin(); it!=this->port_map.end(); ++it) {
//            if (it->second.status == CONNECTED && it->second.nei_id!=source_id){
//                this->sys->send(it->second.port_id, packet, size);
//
//            }
//        }

//        for (int i=0; i<this->num_of_port;i++){
//            if (this->port_status[i].status == CONNECTED && this->port_status[i].nei_id!=source_id){
//                this->sys->send(i, packet, size);
//                this->ls_topology_map.insert();
//            }
//        }
//    }
}

void RoutingProtocolImpl::forward_message_LS(unsigned short port, void *packet,unsigned short size){
    printf("[FORWARD] DATA \n");
}

void RoutingProtocolImpl::flooding_lsp(){
//    unsigned short payload_size = this->num_of_nei*4;
//    unsigned short total_size = payload_size+4*3;
//    char* buffer = (char *) malloc(total_size);
//    *(ePacketType *)(buffer) = (ePacketType) LS;
//    *(unsigned short *) (buffer+2) = htons((unsigned short) total_size);
//    *(unsigned short *) (buffer+4) =htons((unsigned short) this->router_id);
//    *(unsigned int *) (buffer+8) =  htonl(this->ls_seq_num);
//    this->ls_seq_num++;
//    /*add the payload into the packet*/
}
