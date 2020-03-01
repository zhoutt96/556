//
// Created by zhoutt96 on 2/26/20.
//

#include <sys/types.h>

#ifndef PROJECT2_UTILS_H
#define PROJECT2_UTILS_H
#define DEFAULTMAXWINDOWSIZE 65535
#define DATASIZE 1400  // The size of the payload part of sending packet

typedef struct window{
    __uint16_t usable; // total available window
//    __uint16_t unack_start; // sent but not yet acknowledged
//    __uint16_t unack_end; //
    __uint16_t to_be_send; // the index of next to be sent data
    __uint16_t to_be_ack; // the index of next to be acknowledged data
} sendwindow;

typedef struct packet{
    // header
    __uint32_t seq_num;
    __uint32_t ack_num;
    __uint16_t header_checksum;
//    __uint16_t window_size;
    __uint16_t payload_checksum;
    // payload
    char data[DATASIZE];
} packet;

typedef struct ackpacket{
    __uint32_t ack_num;
    __uint16_t ack_checksum;
} ackpacket;

void readFile(char* fullFilePath, char* contentbuffer);
int getFileLength(char* fullFilePath);
u_short cksum(u_short *buf, int count);
void createFile(char* filename);

#endif //PROJECT2_UTILS_H
