//
// Created by zhoutt96 on 4/6/20.
//

#include "utils.h"
#include "RoutingProtocolImpl.h"

void RoutingProtocolImpl::init_port_vector(){
    this->port_status =(PORT*) malloc(sizeof(PORT)*this->num_of_port);
    for (int i=0; i<this->num_of_port; i++){
        this->port_status[i] = PORT(i,this->sys->time());
    }
    this->printPortStatus();
}

void RoutingProtocolImpl::init_ping() {
    printf("create ping message \n");
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
    printf("[RECV] Pong Message, Size is %u \n", size);
    unsigned int ping_time = *(unsigned int *)((char*)packet + 8);
    unsigned int rtt = sys->time() - ntohl(ping_time);
    unsigned short nei_id = *(unsigned int *)((char*)packet + 4);
    this->port_status[port].status = CONNECTED;
    this->port_status[port].last_refreshed_time = this->sys->time();
    this->port_status[port].nei_id = ntohs(nei_id);
    this->port_status[port].link_cost = rtt;
    this->printPortStatus();
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
    for (int i=0; i<this->num_of_port; i++){
        unsigned int duration = sys->time() - this->port_status[i].last_refreshed_time;
        if (duration > 15*1000 && this->port_status[i].status == CONNECTED){
            // this port status is expired
            printf("[Delete Expire] \n");
            this->port_status[i].status = DEAD;
            this->port_status[i].last_refreshed_time = sys->time();
            this->printPortStatus();
        }

        if (this->routing_protocol == P_DV){
            this->updateDV();
        }else if (this->routing_protocol == P_LS){
            this->updateLS();
        }
    }
    this->sys->set_alarm(this, 1000, data);
}

void RoutingProtocolImpl::ping_alarm_handler(void* data) {
    printf("[RECV ALARM] PING \n");
    this->init_ping();
    this->sys->set_alarm(this, 10000, data);
}

void RoutingProtocolImpl::printPortStatus(){
    printf("--------------Port Status Table -----------------\n");
    cout<<"time = "<<this->sys->time()/1000.0<<" ";
    cout<<"Print Port Status on Node "<<this->router_id<<"\n";
    for (int i=0; i<this->num_of_port; i++){
        cout <<"port num is " <<i << ", status is " << this->port_status[i].status << ", neighbor id "<<this->port_status[i].nei_id << ",cost is "<<this->port_status[i].link_cost <<"\n";
    }
    printf("------------------------- End  ------------------\n");
}

