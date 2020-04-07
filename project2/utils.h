//
// Created by zhoutt96 on 2/26/20.
//

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>

#ifndef PROJECT2_UTILS_H
#define PROJECT2_UTILS_H
#define DEFAULTMAXWINDOWSIZE 60
#define DATASIZE 20000  // The size of the payload part of sending packet
#define RESENDLIMIT 3
//#define ERROR_NUM 65534
#define IGNORE_CODE 0
#define FIN 1
#define FINACK 2
#define FIN_TIMEOUT 1000000
#define TIMEEXCEEDLIMIT 1500
#define RESENDTIMELIMIT 1000
#define RECEIVED 1
#define NOTRECEIVED 0

typedef struct window{
    __uint16_t usable; // total available window
    __uint32_t to_be_send; // the index of next to be sent data
//    __uint16_t to_be_ack; // the index of next to be acknowledged data
    int invalid_count;
} sendwindow;

typedef struct packet{
    __uint32_t seq_num;
    __uint16_t payload_size;
    __uint16_t checksum;
    __uint16_t isEnd; // 0 -> has not reached to end, 1->the last packet, 2->filename
    char data[DATASIZE];

} packet;

typedef struct ackpacket{
    __uint32_t ack_num;
    __uint32_t last_inorder_ack;
    __uint16_t ack_checksum;
    __uint16_t isFin;
} ackpacket;

FILE* openFile(char* fullFilePath);
//void readFile(char* fullFilePath, char* contentbuffer, int length);
void readFile(FILE *fp, char* contentbuffer, int length);
int getFileLength(char* fullFilePath);
u_short cksum(u_short *buf, int count);
void createFile(char* filename);
long double calLatency(struct timeval* start, struct timeval* end);
void fillackPacket(ackpacket* ack_packet);
long getLatency(struct timeval* start, struct timeval* end);



#endif //PROJECT2_UTILS_H
