//
// Created by zhoutt96 on 2/5/20.
//

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
#include <dirent.h>

void enterIntoWeb()
{
    printf("Web Server Running");
}

int validDir(char* request_dir){

    if (strstr(request_dir, "../")){   //400 Bad Request
        return 0;
    }
    if(strcmp(request_dir,"/")==0)
        return -1;
    FILE *fp = NULL;
    fp = fopen(request_dir,"r");
    if(fp == NULL)   //404 Not Found
        return -1;
    return 1;   //200 OK

}

//void sendingMsgs(int client_sock, char* err_msg, int msg_size){
//    int cnt = 0;
//    while(cnt < msg_size){
//        cnt += send(client_sock, err_msg+cnt, msg_size-cnt, 0);
//    }
//    printf("sending finished\n");
//}

void readFile(char* fname, int socket)
{
    FILE *fp = NULL;
    char *buffer;
    int filesize;
    int readCount;

    char header[256] = "HTTP/1.1 ";
    int type = validDir(fname);
    printf("%s\n",fname);
    switch(type){
        case 1: strcat(header, "200 OK \r\n");break;
        case 0: strcat(header, "400 Bad Request \r\n");break;
        case -1: strcat(header, "404 Not Found \r\n");break;
    }
    strcat(header,"Content-Type: text/html \r\n");
    strcat(header," \r\n");
    send(socket,header,strlen(header),0);
    printf("%s\n",header);
    if(type!=1){
        printf("something wrong!\n");
        char msg[256] = "<!DOCTYPE html><html><body><h1>";
        strcat(msg,header);
        strcat(msg,"</h1></body></html>");
        int sendCount = send(socket,msg,strlen(msg),0);
        printf("%s\n",msg);
        printf("send count:%d, real size:%d\n",sendCount,strlen(msg));
        if(errno == EAGAIN){
            printf("send error!\n");
        }
        return;
    }

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

    printf("%s", buffer);


//    return buffer;
}

void getFileName(char* buffer, char* rootDirectory)
{
    char *action;
    char *fileName;
    action = (char*) malloc(BUFSIZ);
    fileName = (char*) malloc(BUFSIZ);

    sscanf(buffer, "%s %s",action, fileName);
    strcat(rootDirectory, fileName);
    printf("buff: %s\n",buffer);
    printf("action: %s\n",action);
    printf("fileName: %s\n",fileName);
//    printf("The final file name is %s", rootDirectory);
//    readFile(rootDirectory);
//    return rootDirectory;
}


