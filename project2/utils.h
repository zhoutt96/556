//
// Created by zhoutt96 on 2/26/20.
//

#include <sys/types.h>

#ifndef PROJECT2_UTILS_H
#define PROJECT2_UTILS_H

void readFile(char* fullFilePath, char* contentbuffer);
int getFileLength(char* fullFilePath);
u_short cksum(u_short *buf, int count);

#endif //PROJECT2_UTILS_H


