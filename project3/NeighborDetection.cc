//
// Created by zhoutt96 on 4/6/20.
//

#include "utils.h"
#include "RoutingProtocolImpl.h"
#include <vector>

void RoutingProtocolImpl::init_port_vector(){
    // init the status of each port as unconnected, and init the
//    printf("[INIT] Port Status Array \n");
    this->port_status =(PORT*) malloc(sizeof(PORT)*this->num_of_port);
    for (int i=0; i<this->num_of_port; i++){
        this->port_status[i] = PORT(i);
        this->port_status[i].last_refreshed_time = this->sys->time();
    }
}

void RoutingProtocolImpl::init_ping() {
    char* buffer = (char *) malloc(SIZE_OF_PP);
//    printf("[INIT] Send Ping Message On All Port At the Beginning \n");
    for (int i=0; i<this->num_of_port; i++){
        *(ePacketType *)(buffer) = (ePacketType) PING;
        *(unsigned short *) (buffer+2) = (unsigned short) htons(SIZE_OF_PP);
        *(unsigned short *) (buffer+4) = (unsigned short) htons(this->router_id);
        *(unsigned int *) (buffer+8) = (unsigned int) htonl(this->sys->time());
        sys->send(i, buffer, SIZE_OF_PP);
    }

    alarmType cur_alarm = alarmType::PING_ALARM;
    void *ptr = &cur_alarm;
    this->sys->set_alarm(this, 10000, (void*) ptr);
}

void RoutingProtocolImpl::ping_message_handler(unsigned short port, void *packet, unsigned short size) {
    if (size != SIZE_OF_PP){
//        printf("The size of PING Packet is wrong \n");
        return;
    }
    char* send_buffer = (char *) malloc(SIZE_OF_PP);
//    printf("[RECV] Ping Message, size is %u \n", size);
    unsigned short target_id = *(unsigned short *)((char*)packet + 4);
    *(ePacketType*)(send_buffer) = (ePacketType) PONG;
    *(unsigned short *)((char*)packet +2) = htons(size);
    *(unsigned short *)((char*)packet + 4) = htons(this->router_id);
    *(unsigned short *)((char*)packet + 6) = htons(target_id);
    this->sys->send(port, send_buffer, SIZE_OF_PP);
}

void RoutingProtocolImpl::pong_message_handler(unsigned short port, void *packet, unsigned short size) {
    if (size != SIZE_OF_PP){
//        printf("The size of PONG Packet is wrong \n");
        return;
    }
//    printf("[RECV] Pong Message, size is %u \n", size);
    unsigned int ping_time = *(unsigned int *)((char*)packet + 8);
    unsigned int rtt = sys->time() - ntohl(ping_time);
    unsigned short nei_id = *(unsigned int *)((char*)packet + 4);
    this->port_status[port].status = CONNECTED;
    this->port_status[port].last_refreshed_time = this->sys->time();
    this->port_status[port].nei_id = ntohs(nei_id);
    this->port_status[port].link_cost = rtt;
//    printf("[Finish] Pong Message \n");
}

void RoutingProtocolImpl::data_message_handler() {
//    printf("[RECV] Data Message \n");
}

void RoutingProtocolImpl::init_expire_alarm(){
    alarmType cur_alarm = alarmType::EXPIRE_ALARM;
    void *ptr = &cur_alarm;
    this->sys->set_alarm(this, 1000, ptr);
};

void RoutingProtocolImpl::expire_alarm_handler(){
    /* visit all the elements in port_status_vector, if its status is not refreshed in 15 seconds,
     * then set it as dead */
//    printf("[RECEIVE ALARM] %d \n", EXPIRE_ALARM);
    for (int i=0; i<this->num_of_port; i++){
        unsigned int duration = sys->time() - this->port_status[i].last_refreshed_time;
        if (duration > 15*1000){
            // this port status is expired
            this->port_status[i].status = DEAD;
            this->port_status[i].last_refreshed_time = sys->time();
        }

        if (this->routing_protocol == P_DV){
            this->updateDV();
        }else if (this->routing_protocol == P_LS){
            this->updateLS();
        }
    }
    this->init_expire_alarm();
}

void RoutingProtocolImpl::ping_alarm_handler() {
    alarmType cur_alarm = alarmType::PING_ALARM;
    void *ptr = &cur_alarm;
    this->sys->set_alarm(this, 10000, (void*) ptr);
}

