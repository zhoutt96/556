////
//// Created by zhoutt96 on 2/29/20.
////
//
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
//#include <jmorecfg.h>
#include "utils.h"
//
//#ifndef PROJECT2_QUEUE_H
//#define PROJECT2_QUEUE_H
//
//typedef struct PNode{
////    packet *data;
////    __uint32_t seqNum;
//    __uint32_t data[2]; // the first element is seq_num, the second element is data_index
////    __uint32_t data_index;
////    __uint32_t seq_num;
//    struct PNode * next;
//} PNode;
//
////typedef packet PNode;
//
//typedef struct Queue {
//    PNode* front;
//    PNode* rear;
////    packet* front;
////    packet* rear;
//} Queue;
//
//
////void append(Queue *q, __uint32_t seq_num);
//void append(Queue *q, __uint32_t seq_num, __uint32_t data_length);
////void append(Queue *q, packet new_packet);
//void pop(Queue *q);
//__uint32_t* front(Queue *q);
////__uint32_t front(Queue *q);
//void free_list(Queue *q);
//boolean is_empty(Queue *q);
//
//#endif //PROJECT2_QUEUE_H



#define QUEUE_SIZE DEFAULTMAXWINDOWSIZE

typedef struct Queue
{
    packet* data[QUEUE_SIZE];
    int front;
    int rear;
}Queue;

void InitQueue(Queue* queue){
    queue->front = 0;
    queue->rear = 0;
    int i;
    for (i=0; i<QUEUE_SIZE;i++){
        queue->data[i] = (packet*) malloc(sizeof(packet));
    }
}

//Queue *InitQueue()
//{
//    Queue *q = (Queue *)malloc(sizeof(Queue));
//    if(q == NULL)
//    {
//        printf("Malloc failed!\n");
//        exit(-1);
//    }
//    q->front = 0;
//    q->rear = 0;
//
//    return q;
//}

int IsFull(Queue *q)
{
    return ((q->rear+1)%QUEUE_SIZE == q->front);
}

int IsEmpty(Queue *q)
{
    return (q->front == q->rear);
}

//__uint32_t seq_num;
////    __uint32_t ack_num;
//__uint16_t header_checksum;
////    __uint16_t window_size;
////    __uint16_t payload_checksum;
//
//__uint16_t payload_size;
//__uint16_t payload_checksum;

//void Enqueue(Queue *q, __uint32_t seq_num, __uint16_t header_checksum, __uint16_t payload_size, __uint16_t payload_checksum)
//{
//    if(IsFull(q))
//    {
//        return;
//    }
//
//    q->data[q->rear]->data = data;
//    q->data[q->rear]->data = data;
//
//    q->rear = (q->rear+1)%QUEUE_SIZE;
//}

//void Enqueue(Queue *q, __uint32_t seq_num, __uint16_t header_checksum, __uint16_t payload_size,  __uint16_t payload_checksum)
void Enqueue(Queue *q)
{
    if(IsFull(q))
    {
        return;
    }

//    q->data[q->rear]->seq_num = seq_num;
//    q->data[q->rear]->header_checksum = header_checksum;
//    q->data[q->rear]->payload_size = payload_size;
//    q->data[q->rear]->payload_checksum = header_checksum;

    q->rear = (q->rear+1)%QUEUE_SIZE;
}

//int Dequeue(Queue *q)
//{
//    if(IsEmpty(q))
//    {
//        return 0;
//    }
//
//    int tmp = q->data[q->front];
//    q->front = (q->front+1)%QUEUE_SIZE;
//    return tmp;
//}

packet* Rear(Queue *q){
    return q->data[q->rear];
}

packet* Dequeue(Queue *q)
{
    if(IsEmpty(q))
    {
        return 0;
    }

    packet* tmp = q->data[q->front];
    q->front = (q->front+1)%QUEUE_SIZE;
    return tmp;
}

packet* Front(Queue *q){
    return q->data[q->front];
}

