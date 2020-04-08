#include "RoutingProtocolImpl.h"
#include "utils.h"

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
    this->init_port_vector();
    this->init_ping();
    this->init_expire_alarm();
    if (protocol_type == P_DV){
      this->init_DV_Protocol(); // to be finished
    }else if(protocol_type == P_LS){
      this->init_LS_Protocol(); // to be finished
    }
}

void RoutingProtocolImpl::handle_alarm(void* data) {
    alarmType cur_type = *((alarmType*)data);

    printf("[RECV ALARM] %d \n", cur_type);
    switch (cur_type){
        case PING_ALARM:
            this->init_ping();
            break;
        case DV_ALARM:
            this->updateDV(); // to be finished
            break;
        case LS_ALARM:
            this->updateLS(); // to be finished
            break;
        case EXPIRE_ALARM:
            this->expire_alarm_handler();
            break;
        default:
            printf("Can not recognize the alarm \n");
            return;
    }
}

void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
    unsigned short actual_size =  *(unsigned short *)((char*)packet + 2);
    if (ntohs(actual_size) != size){
//        printf("[ERROR] Packet Size is wrong \n");
        return;
    }
    ePacketType cur_type = (ePacketType)(*((unsigned char *)packet));
    printf("the type is %d, size is %u\n", cur_type, size);
    switch (cur_type){
        case DATA:
            this->data_message_handler();
        case PING:
            this->ping_message_handler(port, packet, size);
        case PONG:
            this->pong_message_handler(port, packet, size);
        case DV:
            this->DV_message_handler(); // to be finished
        case LS:
            this->LS_message_handler(); // to be finished
    }
}


// add more of your own code
