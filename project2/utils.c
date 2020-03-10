//
// Created by zhoutt96 on 2/26/20.
//
#include <errno.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/types.h>
//#include <unistd.h>
#include "utils.h"

#define STATUS200 200
#define ERROR404 404
#define ERROR400 400
#define ERROR500 500
#define ERROR501 501

int getFileLength(char* fullFilePath)
{
    FILE *fp = NULL;
    int filesize;

    fp = fopen(fullFilePath, "r");
    if (fp == NULL)
    {
        perror("Can not find this file");
        abort();
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
    return filesize;
}

FILE* openFile(char* fullFilePath){
    FILE *fp = NULL;
    fp = fopen(fullFilePath, "r");
    if (fp == NULL)
    {
        perror("Can not find this file");
        abort();
    }
    return fp;
}

void readFile(FILE *fp, char* buffer, int length){
    int readCount = fread (buffer,1,length,fp);
//    if (readCount != length)
//    {
//        perror("Reading Error");
//        abort();
//    }
}

//void readFile(char* fullFilePath, char* buffer, int length){
//    FILE *fp = NULL;
//    int filesize;
//    int readCount;
//
//    fp = fopen(fullFilePath, "r");
//    if (fp == NULL)
//    {
//        perror("Can not find this file");
//        abort();
//    }
//
//    fseek(fp, 0, SEEK_END);
//    filesize = ftell(fp);
////    buffer = (char*) malloc(filesize);
//
//    fseek(fp, 0, SEEK_SET);
//
//    readCount = fread (buffer,1,filesize,fp);
//    if (readCount != filesize)
//    {
//        perror("Reading Error");
//        abort();
//    }
//
//    if(errno == EAGAIN){
//        printf("send error!\n");
//    }
//}


u_short cksum(u_short *buf, int count)
{
    register u_long sum = 0;
    while (count--)
    {
        sum += *buf++;
        if (sum & 0xFFFF0000)
        {
            /* carry occurred, so wrap around */
            sum &= 0xFFFF;
            sum++;
        }
    }
    return ~(sum & 0xFFFF);
}

long double calLatency(struct timeval* start, struct timeval* end){
    return (end->tv_sec - start->tv_sec)*1000000 + end->tv_usec - start->tv_usec;
}
