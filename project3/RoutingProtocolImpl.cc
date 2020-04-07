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

void RoutingProtocolImpl::handle_alarm(void *data) {
    alarmType *cur_type = (alarmType*) data;
    printf("receive the alarm type of %d \n", *cur_type);
    switch (*cur_type){
        case PING_ALARM:
            this->init_ping();
        case DV_ALARM:
            this->updateDV(); // to be finished
        case LS_ALARM:
            this->updateLS(); // to be finished
        case EXPIRE_ALARM:
            this->expire_alarm_handler();
    }
}

void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
    Packet *recv_packet = (Packet*) malloc(sizeof(Packet));
    unsigned char type = recv_packet->type;
    switch (type){
        case DATA:
            this->data_message_handler();
        case PING:
            this->ping_message_handler(port, recv_packet, size);
        case PONG:
            this->pong_message_handler(port, recv_packet, size);
        case DV:
            this->DV_message_handler(); // to be finished
        case LS:
            this->LS_message_handler(); // to be finished
    }
}


// add more of your own code
