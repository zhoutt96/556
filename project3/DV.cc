//
// Created by zhoutt96 on 4/6/20.
//


//#include "utils.h"
#include "RoutingProtocolImpl.h"

void RoutingProtocolImpl::init_DV_alarm(){

}

void RoutingProtocolImpl::updateDV(){
//    printf("Update DV now \n");


}

void RoutingProtocolImpl::init_DV_Protocol(){
//    printf("Init DV now \n");
}

void RoutingProtocolImpl::DV_message_handler(unsigned short port, void *packet,unsigned short size){
    printf("[RECV] DV Message \n");
}

void RoutingProtocolImpl::forward_message_DV(unsigned short port, void *packet, unsigned short size) {
    printf("[Forward] DATA \n");
}

