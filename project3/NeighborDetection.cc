//
// Created by zhoutt96 on 4/6/20.
//

#include "utils.h"
#include "RoutingProtocolImpl.h"
#include <vector>


void RoutingProtocolImpl::init_port_vector(){
    // init the status of each port as unconnected, and init the
    printf("[INIT] Port Status Array \n");
    this->port_status =(PORT*) malloc(sizeof(PORT)*this->num_of_port);
    for (int i=0; i<this->num_of_port; i++){
        this->port_status[i] = PORT(i);
        this->port_status[i].last_refreshed_time = this->sys->time();
    }
}

void RoutingProtocolImpl::init_ping() {
    /*ping message does not have a destination ID*/
    printf("[INIT] Send Ping Message On All Port At the Beginning \n");
    for (int i=0; i<this->num_of_port; i++){
        Packet *ping_packet = (Packet*) malloc(SIZE_OF_PP);
        ping_packet->source_id = htons(this->router_id);
        ping_packet->size = htons(SIZE_OF_PP);
        ping_packet->type = htons(PING);
        ping_packet->payload = (char*) malloc(sizeof(unsigned int));
        *(unsigned int *) ping_packet->payload = htonl(this->sys->time());
        sys->send(i, ping_packet, SIZE_OF_PP);
    }
    alarmType ping_alarm = PING_ALARM;
    printf("[ALARM TYPE] Init : %d \n", ping_alarm);
    this->sys->set_alarm(this, 10000, (void*) ping_alarm);
}

void RoutingProtocolImpl::ping_message_handler(unsigned short port, Packet *recv_packet, unsigned short size) {
    /*when the neighbor router receives the PING message, it mush update the received message's type to PONG
     * copy the source ID to the destination ID, update the source ID to its own, then send the resulting PONG
     * message(with the original timestamp still in the payload) immediately back to the neighbor*/
    printf("[RECV] Ping Message \n");
    recv_packet->des_id = htons(recv_packet->source_id);
    recv_packet->source_id = htons(this->router_id);
    this->sys->send(port, (void*) recv_packet, size);
}

void RoutingProtocolImpl::pong_message_handler(unsigned short port, Packet *recv_packet, unsigned short size) {
    printf("[RECV] Pong Message \n");
    unsigned int ping_time = *(unsigned int*) recv_packet->payload;
    unsigned int rtt = sys->time() - ntohl(ping_time);
    unsigned short nei_id = recv_packet->source_id;
    this->port_status[port].status = CONNECTED;
    this->port_status[port].last_refreshed_time = this->sys->time();
    this->port_status[port].nei_id = ntohs(nei_id);
    this->port_status[port].link_cost = rtt;
}

void RoutingProtocolImpl::data_message_handler() {
    printf("[RECV] Data Message \n");
}


void RoutingProtocolImpl::init_expire_alarm(){
    alarmType EXPIRE_alarm = EXPIRE_ALARM;
    printf("[SET ALARM] %d \n", EXPIRE_alarm);
    this->sys->set_alarm(this, 1000, (void*) EXPIRE_alarm);
};

void RoutingProtocolImpl::expire_alarm_handler(){
    /* visit all the elements in port_status_vector, if its status is not refreshed in 15 seconds,
     * then set it as dead */
    printf("[RECEIVE ALARM] %d \n", EXPIRE_ALARM);
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
}

