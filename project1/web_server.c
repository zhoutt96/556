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

void enterIntoWeb()
{
    printf("Web Server Running");
}




void readFile(char* fname, int socket)
{
    FILE *fp = NULL;
    char *buffer;
    int filesize;
    int readCount;

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


