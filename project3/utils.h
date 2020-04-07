//
// Created by zhoutt96 on 4/6/20.
//


#ifndef PROJECT3_UTILS_H
#define PROJECT3_UTILS_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>


#define SIZE_OF_PP 12
typedef enum alarmType
{
    DV_ALARM,
    LS_ALARM,
    PING_ALARM,
    EXPIRE_ALARM
}alarmType;

typedef enum portStatus
{
    CONNECTED,
    UNCONNECTED,
    DEAD
}portStatus;

typedef struct Packet{
    unsigned char type;
    unsigned char reserved;
    unsigned short size;
    unsigned short source_id;
    unsigned short des_id;
    char* payload;
} Packet;

typedef struct PORT{
    unsigned char status;
    unsigned int last_refreshed_time;
    unsigned int port_id;
    unsigned short nei_id;
    unsigned int link_cost; // RRT

    PORT( unsigned int port_id){
        this->status = UNCONNECTED;
        this->port_id = port_id;
    }
}PORT;

//typedef struct PP_Packet{
//    unsigned char type;
//    unsigned char reserved;
//    unsigned short size;
//    unsigned short source_id;
//    unsigned short des_id;
//    unsigned int payload;
//} PP_Packet;



#endif //PROJECT3_UTILS_H
