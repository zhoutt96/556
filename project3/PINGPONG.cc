

#ifndef PROJECT3_DETECTNEIGHBOR_CC
#define PROJECT3_DETECTNEIGHBOR_CC

#include "utils.h"
#include "RoutingProtocolImpl.h"

int PingPong_size = 12;

void RoutingProtocolImpl::init_ping() {
//    printf("create ping message \n");
    for (int i=0; i<this->num_of_port; i++){
        char* buffer = (char *) malloc(SIZE_OF_PP);
        *(ePacketType *)(buffer) = (ePacketType) PING;
        *(unsigned short *) (buffer+2) = (unsigned short) htons(SIZE_OF_PP);
        *(unsigned short *) (buffer+4) = (unsigned short) htons(this->router_id);
        *(unsigned int *) (buffer+8) = (unsigned int) htonl(this->sys->time());
        sys->send(i, buffer, SIZE_OF_PP);
    }
}

void RoutingProtocolImpl::init_ping_alarm(){
    void* ptr = malloc(sizeof(alarmType));
    *((alarmType*)ptr) = PING_ALARM;
    this->sys->set_alarm(this, 10000, ptr);
}

void RoutingProtocolImpl::ping_message_handler(unsigned short port, void *packet, unsigned short size) {
    printf("[RECV] Ping Message, Size is %u \n", size);
    unsigned short target_id = *(unsigned short *)((char*)packet + 4);
    *(ePacketType*)(packet) = (ePacketType) PONG;
    *(unsigned short *)((char*)packet +2) = htons(size);
    *(unsigned short *)((char*)packet + 4) = htons(this->router_id);
    *(unsigned short *)((char*)packet + 6) = htons(target_id);
    this->sys->send(port, packet, SIZE_OF_PP);
}

void RoutingProtocolImpl::pong_message_handler(unsigned short port, void *packet, unsigned short size) {

    int update = 0;
    unsigned int ping_time = *(unsigned int *)((char*)packet + 8);
    unsigned int rtt = sys->time() - ntohl(ping_time);
    unsigned short nei_id = ntohs(*(unsigned short *)((char*)packet + 4));
    printf("[RECV] Pong Message, on node %u from node %u \n", router_id, nei_id);

    if (!port_map.count(nei_id) || port_map[nei_id].status!=CONNECTED || port_map[nei_id].port_id!=port || port_map[nei_id].link_cost!=rtt){
        update = 1;
    }

    this->port_map[nei_id].last_refreshed_time = this->sys->time();
    this->port_map[nei_id].port_id = port;
    this->port_map[nei_id].status = CONNECTED;
    this->port_map[nei_id].nei_id = nei_id;
    this->port_map[nei_id].link_cost = rtt;
    if (update){
        // detect the new neighbour
        if (routing_protocol == P_LS){
            this->flooding_lsp();
        }else{
            updateDV();
        }
    }

    this->printPortStatus();
    this->print_flooding_table();
}

void RoutingProtocolImpl::data_message_handler(unsigned short port, void *packet,unsigned short size) {
    printf("[RECV] Data Message \n");
    if (this->routing_protocol == P_LS){
        this->forward_message_DV(port, packet, size);
    }else if(this->routing_protocol == P_DV){
        this->forward_message_LS(port, packet, size);
    }
}

void RoutingProtocolImpl::init_expire_alarm(){
    void* ptr = malloc(sizeof(alarmType));
    *((alarmType*)ptr) = EXPIRE_ALARM;
    this->sys->set_alarm(this, 1000, ptr);
};

void RoutingProtocolImpl::expire_alarm_handler(void* data){
    printf("[RECV ALARM]EXPIRE \n");
    int updated=0;
    for (auto it=this->port_map.begin(); it!=this->port_map.end(); it++){
        unsigned int last_refreshed_time = it->second.last_refreshed_time;
        unsigned int duration = sys->time() - last_refreshed_time;

        if (duration >= 15*1000 && this->port_map[it->first].status == CONNECTED){
            printf("EXPIRE \n");
            this->port_map[it->first].status = UNCONNECTED;
            this->port_map[it->first].last_refreshed_time = this->sys->time();
            this->delete_nei_in_lsp(it->second.nei_id);
            updated = 1;
            printPortStatus();
        }
    }

    if (updated == 1){
        if (this->routing_protocol == P_DV){
            this->updateDV();
        }else if (this->routing_protocol == P_LS){
//            this->updateLS();
//            this->delete_nei_in_lsp();
            this->flooding_lsp();
        }
    }
    LS_expire_alarm_handler(data);
    this->sys->set_alarm(this, 1000, data);
}

void RoutingProtocolImpl::ping_alarm_handler(void* data) {
    this->init_ping();
    this->sys->set_alarm(this, 10000, data);
}

void RoutingProtocolImpl::printPortStatus(){
    printf("\n------------------Port Status Table ---------------------\n");
    cout<<"time = "<<this->sys->time()/1000.0<<" ";
    cout<<"Print Port Status on Node "<<this->router_id<<"\n";
    for (auto it=this->port_map.begin(); it!=this->port_map.end(); ++it) {
        cout <<"port num is " <<it->second.port_id<< ", status is " << it->second.status << ", neighbor id "<< it->second.nei_id << ",cost is "<< it->second.link_cost <<"\n";
    }
    printf("----------------------------- End  ----------------------\n");
}

int RoutingProtocolImpl::get_nei_num(){
    int num_nei=0;
    for (auto it=this->port_map.begin(); it!=this->port_map.end(); ++it) {
        if (it->second.status == CONNECTED){
            num_nei++;
        }
    }
    return num_nei;
}



#endif //PROJECT3_DETECTNEIGHBOR_CC
