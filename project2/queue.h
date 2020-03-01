//
// Created by zhoutt96 on 2/29/20.
//

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <jmorecfg.h>

#ifndef PROJECT2_QUEUE_H
#define PROJECT2_QUEUE_H

typedef struct PNode{
    __uint32_t data[2]; // the first element is seq_num, the second element is data_length
    struct PNode * next;
} PNode;

typedef struct Queue{
    PNode* front;
    PNode* rear;
}Queue;

void append(Queue *q, __uint32_t seq_num, __uint32_t data_length);
void pop(Queue *q);
__uint32_t* front(Queue *q);
void free_list(Queue *q);
boolean is_empty(Queue *q);

#endif //PROJECT2_QUEUE_H
