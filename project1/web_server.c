//
// Created by zhoutt96 on 2/5/20.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h>
#include <unistd.h>

#define STATUS200 200
#define ERROR404 404
#define ERROR400 400
#define ERROR500 500
#define ERROR501 501

int validDir(char* request_dir, char *action){
//    printf("The action is %s", action);
    if (strcmp(action, "GET") != 0){
//        printf("Get ERROR 501");
        return ERROR501;
    }
    if (strstr(request_dir, "../")){   //400 Bad Request
        return ERROR400;
    }
    if(request_dir[strlen(request_dir)-1]=='/') // 400 cannot be directory
        return ERROR400;
//    FILE *fp = NULL;
//    fp = fopen(request_dir,"r");
//    if(fp == NULL)   //404 Not Found
//        return ERROR404;
    if(access(request_dir,F_OK)!= 0)
        return ERROR404;
    if(access(request_dir,R_OK)!=0)
        return ERROR500;
    return STATUS200;   //200 OK
}

void sendResponse(char* fname, int socket, char* action)
{
    FILE *fp = NULL;
    char *buffer;
    int filesize;
    int readCount;

    char header[256] = "HTTP/1.1 ";
    int type = validDir(fname, action);

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
        case ERROR501:
            fname = "./static/501.html";
            strcat(header, "501 Not Implemented \r\n");
        case ERROR500:
            fname = "./static/500.html";
            strcat(header, "500 Internal Server Error \r\n");
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

void extractInfoFromHeader(char* buffer, char* rootDirectory, char* action)
{
    char *fileName;
    fileName = (char*) malloc(BUFSIZ);
    sscanf(buffer, "%s %s",action, fileName);
    strcat(rootDirectory, fileName);
}

void getRequestAction(char *buffer){

}

