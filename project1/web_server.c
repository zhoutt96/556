//
// Created by zhoutt96 on 2/5/20.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h>

#define STATUS200 200
#define ERROR404 404
#define ERROR400 400
#define ERROR500 500
#define EROOR501 501

int validDir(char* request_dir){

    if (strstr(request_dir, "../")){   //400 Bad Request
        return ERROR400;
    }
    if(strcmp(request_dir,"/")==0) // ***********????????
//        printf("Get a 400 ERROR");
        return ERROR400;
    FILE *fp = NULL;
    fp = fopen(request_dir,"r");
    if(fp == NULL)   //404 Not Found
        return ERROR404;
    return STATUS200;   //200 OK
}

void readFile(char* fname, int socket)
{
    FILE *fp = NULL;
    char *buffer;
    int filesize;
    int readCount;

    char header[256] = "HTTP/1.1 ";
    int type = validDir(fname);
    printf("The type is %d \n", type);
    switch(type){
        case STATUS200:
            strcat(header, "200 OK \r\n");
            break;
        case ERROR400:
            fname = "./static/400.html";
            strcat(header, "400 Bad Request \r\n");
            break;
        case ERROR404:
            fname = "./static/404.html";
            strcat(header, "404 Not Found \r\n");
            break;
    }

    strcat(header,"Content-Type: text/html \r\n");
    strcat(header," \r\n");

    send(socket,header,strlen(header),0);
    fp = fopen(fname, "r");
    if (fp == NULL)
    {
        perror("Can not find this file");
        abort();
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    buffer = (char*) malloc(filesize);

    fseek(fp, 0, SEEK_SET);

    readCount = fread (buffer,1,filesize,fp);
    if (readCount != filesize)
    {
        perror("Reading Error");
        abort();
    }

    send(socket, buffer, filesize, 0);
    if(errno == EAGAIN){
        printf("send error!\n");
    }
}

//void conconateFileName(char* buffer, char* )

void getFileName(char* buffer, char* rootDirectory)
{
    char *action;
    char *fileName;
    action = (char*) malloc(BUFSIZ);
    fileName = (char*) malloc(BUFSIZ);
    sscanf(buffer, "%s %s",action, fileName);
    strcat(rootDirectory, fileName);
}

