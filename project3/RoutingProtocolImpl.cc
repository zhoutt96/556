#include "RoutingProtocolImpl.h"

RoutingProtocolImpl::RoutingProtocolImpl(Node *n) : RoutingProtocol(n) {
    sys = n;
}

RoutingProtocolImpl::~RoutingProtocolImpl() {
    // add your own code (if needed)
}

void RoutingProtocolImpl::init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type) {
    this->routing_protocol = protocol_type;
    this->num_of_port = num_ports;
    this->router_id = router_id;
    this->init_ping();
    this->init_ping_alarm();
    this->init_expire_alarm();
    this->num_of_nei = 0;
    if (protocol_type == P_DV){
        this->init_DV_Protocol(); // to be finished
    }else if(protocol_type == P_LS){
        this->init_LS_Protocol(); // to be finished
    }
}

void RoutingProtocolImpl::handle_alarm(void *data) {
    alarmType cur_type = *((alarmType*)data);
    switch (cur_type){
        case PING_ALARM:
            this->ping_alarm_handler(data);
            break;
        case DV_ALARM:
            this->updateDV(); // to be finished
            break;
        case LS_ALARM:
            this->updateLS(); // to be finished
            break;
        case EXPIRE_ALARM:
            this->expire_alarm_handler(data);
            break;
        default:
            printf("Can not recognize the alarm \n");
            break;
    }
}

void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
    unsigned short actual_size =  *(unsigned short *)((char*)packet + 2);
    if (ntohs(actual_size) != size){
        printf("[ERROR] Packet Size is wrong \n");
        return;
    }

    ePacketType cur_type = (ePacketType)(*((unsigned char *)packet));
//    printf("the type is %d, size is %u\n", cur_type, size);

    switch (cur_type){
        case DATA:
            this->data_message_handler(port, packet, size);
            break;
        case PING:
            this->ping_message_handler(port, packet, size);
            break;
        case PONG:
            this->pong_message_handler(port, packet, size);
            break;
        case DV:
            this->DV_message_handler(port, packet, size); // to be finished
            break;
        case LS:
            this->LS_message_handler(port, packet, size); // to be finished
            break;
        default:
            printf("[ERROR] Can Not Recognize this Packet Type \n");
            break;
    }
}
