//
// Created by zhoutt96 on 2/5/20.
//

#ifndef PROJECT2_WEB_SERVER_H
#define PROJECT2_WEB_SERVER_H

int validDir(char* request_dir, char *action);
void extractInfoFromHeader(char* buffer, char* rootDirectory, char* action);
void sendResponse(char* fname, int socket, char* action);

#endif //PROJECT2_WEB_SERVER_H

