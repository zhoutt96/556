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
    if(opendir(request_dir) == NULL)   //404 Not Found
        return -1;
    return 1;   //200 OK

}


void readFile(char* fname, int socket)
{
    FILE *fp = NULL;
    char *buffer;
    int filesize;
    int readCount;

    char header[256] = "HTTP/1.1 ";
    int type = validDir(fname);
    switch(type){
        case 1: strcat(header, "200 OK \r\n");
            strcat(header,"Content-Type: text/html \r\n");break;
        case 0: strcat(header, "400 Bad Request \r\n");break;
        case -1: strcat(header, "404 Not Found \r\n");break;
    }
    strcat(header," \r\n");
    send(socket,header,strlen(header),0);
    if(type!=1)
        return;

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

//    printf("%s", buffer);
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
//    printf("The final file name is %s", rootDirectory);
//    readFile(rootDirectory);
//    return rootDirectory;
}


