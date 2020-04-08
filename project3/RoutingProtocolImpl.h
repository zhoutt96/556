#ifndef ROUTINGPROTOCOLIMPL_H
#define ROUTINGPROTOCOLIMPL_H



#include "global.h"
#include "Event.h"
#include "Link.h"
#include "Node.h"
#include "Simulator.h"
#include "RoutingProtocol.h"
#include "utils.h"
#include <vector>


class RoutingProtocolImpl : public RoutingProtocol {
    private:
        unsigned short num_of_port;
        unsigned short router_id;
        eProtocolType routing_protocol;
        PORT* port_status;

    public:
        RoutingProtocolImpl(Node *n);
        ~RoutingProtocolImpl();

    void init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type);
    // As discussed in the assignment document, your RoutingProtocolImpl is
    // first initialized with the total number of ports on the router,
    // the router's ID, and the protocol type (P_DV or P_LS) that
    // should be used. See global.h for definitions of constants P_DV
    // and P_LS.

    void handle_alarm(void *data);
    // As discussed in the assignment document, when an alarm scheduled by your
    // RoutingProtoclImpl fires, your RoutingProtocolImpl's
    // handle_alarm() function will be called, with the original piece
    // of "data" memory supplied to set_alarm() provided. After you
    // handle an alarm, the memory pointed to by "data" is under your
    // ownership and you should free it if appropriate.

    void recv(unsigned short port, void *packet, unsigned short size);
    // When a packet is received, your recv() function will be called
    // with the port number on which the packet arrives from, the
    // pointer to the packet memory, and the size of the packet in
    // bytes. When you receive a packet, the packet memory is under
    // your ownership and you should free it if appropriate. When a
    // DATA packet is created at a router by the simulator, your
    // recv() function will be called for such DATA packet, but with a
    // special port number of SPECIAL_PORT (see global.h) to indicate
    // that the packet is generated locally and not received from 
    // a neighbor router.


    // functions for init
    void init_ping(); // send ping message at the beginning
    void init_DV_Protocol();
    void init_LS_Protocol();
    void init_port_vector();
    void init_expire_alarm();

    // message handler (five kinds of message in total)
    void ping_message_handler(unsigned short port, void *packet,unsigned short size);
    void pong_message_handler(unsigned short port, void *packet,unsigned short size);
    void DV_message_handler(unsigned short port, void *packet,unsigned short size);
    void LS_message_handler(unsigned short port, void *packet,unsigned short size);
    void data_message_handler(unsigned short port, void *packet,unsigned short size);

    // function to update the DV and LS forwarding table periodically
    void updateDV();
    void updateLS();

    // function to forward data message
    void forward_message_LS(unsigned short port, void *packet,unsigned short size);
    void forward_message_DV(unsigned short port, void *packet,unsigned short size);

    // function to handler the alarm message
    void ping_alarm_handler();
    void expire_alarm_handler();

    // functions for debugging/printing
    void printPortStatus();

 private:
    Node *sys; // To store Node object; used to access GSR9999 interfaces 
};

#endif

