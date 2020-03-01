//
// Created by zhoutt96 on 2/29/20.
//

#include "queue.h"


void append(Queue *q, __uint32_t seq_num, __uint32_t data_length){
    PNode* curnode = (PNode*)malloc(sizeof(PNode));
    curnode->data[0] = seq_num;
    curnode->data[1] = data_length;
    curnode->next = NULL;
    if (q->front==NULL){
        q->front = curnode;
    }else if (q->rear==NULL){
        q->rear = curnode;
    }else{
        q->rear->next = curnode;
        q->rear = curnode;
    }
};

void pop(Queue *q){
    // pop from the front
    if (q->front == q->rear){
        q->front = NULL;
        q->rear = NULL;
    }else{
        q->front = q->front->next;
    }
};

__uint32_t* front(Queue *q){
    return q->front->data;
};

void free_list(Queue *q){
    PNode *tmp;
    while (q->front!=q->rear){
        tmp = q->front;
        q->front = tmp;
        free(tmp);
    }
};

boolean is_empty(Queue *q){
    if (q==NULL){
        return TRUE;
    }else{
        return FALSE;
    }
};

