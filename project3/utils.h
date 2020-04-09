//
// Created by Zequn Jiang on 3/30/20.
//

#ifndef PROJECT3_COMMON_H
#define PROJECT3_COMMON_H

#include "global.h"
#include <netinet/in.h>


#define SIZE_OF_PP 12
#define PACKET_BASE_SIZE 12

typedef std::pair<unsigned short, unsigned int> topology_pair;

enum alarmType
{
    DV_ALARM,
    LS_ALARM,
    PING_ALARM,
    EXPIRE_ALARM
};

typedef enum portStatus
{
    CONNECTED,
    UNCONNECTED,
}portStatus;

typedef struct Packet{
    unsigned char type;  //1
    unsigned char reserved; //1
    unsigned short size; //2
    unsigned short source_id; //2
    unsigned short des_id; //2
    char* payload; //??
} Packet;

struct Topology_Info{
    unsigned short nei_id;
    unsigned short port_id;
    unsigned int cost;

    Topology_Info& operator =(const Topology_Info& a)
    {
        nei_id = a.nei_id;
        port_id = a.port_id;
        cost = a.cost;
        return *this;
    }

    inline bool operator==(const Topology_Info& other) const {
        if (nei_id == other.nei_id)
            return true;
        return false;
    }

    bool operator<(const Topology_Info& other) {
        if (nei_id < other.nei_id )
            return true;
        return false;
    }

    Topology_Info(unsigned short nei_id, unsigned short port_id, unsigned int cost){
        this->nei_id = nei_id;
        this->port_id = port_id;
        this->cost = cost;
    }

};

class MyHashFunction {
    public:
        // id is returned as hash function
        size_t operator()(const Topology_Info& t) const
        {
            return t.nei_id;
        }
};

struct PORT{
    unsigned short status;
    unsigned int last_refreshed_time;
    unsigned int port_id;
    unsigned short nei_id;
    unsigned int link_cost; // RRT

    PORT(){
        this->status = UNCONNECTED;
    }

    PORT( unsigned int port_id){
        this->status = UNCONNECTED;
        this->port_id = port_id;
    }

    PORT(unsigned int port_id, unsigned int last_refresh_time){
        this->port_id = port_id;
        this->last_refreshed_time = last_refresh_time;
        this->status = UNCONNECTED;
    }

//    PORT& operator =(const PORT& a)
//    {
//        port_id = a.port_id;
//        last_refreshed_time = a.last_refreshed_time;
//        status = a.status;
//        link_cost = a.link_cost;
//        nei_id = a.nei_id;
//        return *this;
//    }
//
//    bool operator ==(const PORT& a) const
//    {
//        return(a.port_id == port_id);
//    }

};
#endif //PROJECT3_COMMON_H
