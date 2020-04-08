//
// Created by zhoutt96 on 4/6/20.
//

#include "utils.h"
#include "RoutingProtocolImpl.h"

void RoutingProtocolImpl::updateLS(){
    printf("[Update] LS Table \n");
}

void RoutingProtocolImpl::init_LS_Protocol(){
//    printf("Init LS now \n");
}

void RoutingProtocolImpl::LS_message_handler(unsigned short port, void *packet,unsigned short size){
    printf("[RECV] LS Message \n");
}

void RoutingProtocolImpl::forward_message_LS(unsigned short port, void *packet,unsigned short size){
    printf("[FORWARD] DATA \n");
}