#include "RoutingProtocolImpl.h"

RoutingProtocolImpl::RoutingProtocolImpl(Node *n) : RoutingProtocol(n) {
    sys = n;
}

RoutingProtocolImpl::~RoutingProtocolImpl() {
    // add your own code (if needed)
}

void RoutingProtocolImpl::init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type) {
    // add your own code
    this->num_ports = num_ports;
    this->router_id = router_id;
    this->protocol_type = protocol_type;
    char *PING_PONG_ALARM = new char[sizeof(char) * sizeof(alarmType)];
    char *DV_ALARM = new char[sizeof(char) * sizeof(alarmType)];
    char *EXPIRE_ALARM = new char[sizeof(char) * sizeof(alarmType)];
    *((alarmType *)PING_PONG_ALARM) = A_PING_PONG;
    *((alarmType *)DV_ALARM) = A_DV;
    *((alarmType *)EXPIRE_ALARM) = A_EXPIRE;
    createPingPongMessage();
    sys->set_alarm(this, 1000, EXPIRE_ALARM);
    sys->set_alarm(this, 10 * 1000, PING_PONG_ALARM);
    sys->set_alarm(this, 30 * 1000, DV_ALARM);
    if (protocol_type == P_DV) {
        DVM.init(sys, router_id, num_ports, &neighbors, &ports, &forwarding_table);
    }
    else if (protocol_type == P_LS) {
        LSM.init(sys, router_id, num_ports, &neighbors, &ports, &forwarding_table);
    }
}

void RoutingProtocolImpl::handle_alarm(void *data) {
    // handle type
    alarmType type = (*((alarmType*)data));
    switch (type) {
        case A_PING_PONG:
            createPingPongMessage();
            sys->set_alarm(this, 10 * 1000, data);
            break;
        case A_DV:
            if (protocol_type == P_DV) {
                DVM.sendUpdatePacket();
            } else if (protocol_type == P_LS) {
                LSM.floodLSP();
            }
            sys->set_alarm(this, 30 * 1000, data);
            break;
        case A_EXPIRE:
            if (protocol_type == P_DV) {
                DVM.refresh();
            } else if (protocol_type == P_LS) {
                LSM.checkTimeoutEntry();
            }
            sys->set_alarm(this, 1000, data);
            break;
        default:
            cout << "unexpected type " << type << endl;
            exit(1);
    }
}

void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
    auto type = getPacketType(packet);
    printf("TYPE is %u, SIZE is %u \n", type, size);
    switch (type) {
        case DV:
            DVM.receivePacket(packet, port, size);
            break;
        case LS:
            LSM.recvLSP(packet, size);
            break;
        case PING:
        case PONG:
            handleMessage(port, packet, size);
            break;
        case DATA:
            forwardData(port, packet);
            break;
        default:
            cout << "unexpected type " << type << endl;
            exit(1);
    }
}

void RoutingProtocolImpl::forwardData(unsigned short port, void *packet) {
    checkType(packet, DATA);
    auto start = (unsigned short *)packet;
    unsigned short dest_id = ntohs(*(start + 3));
    if (dest_id == this->router_id) {
        //cout << "data packet receive destination at " << router_id << endl;
        delete[] (char *)packet;
        return;
    }
    if (forwarding_table.find(dest_id) == forwarding_table.end()) {
        cout << "don't know how to send data" << endl;
        return;
    }
    auto next_hop = forwarding_table[dest_id];
    sys->send(neighbors[next_hop].port, packet, getSize(packet));
}
