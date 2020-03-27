
#define QUEUE_SIZE DEFAULTMAXWINDOWSIZE

typedef struct Queue
{
    int size;
    packet* data[QUEUE_SIZE];
    int front;
    int rear;
}Queue;

void InitQueue(Queue* queue){
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
    int i;
    for (i=0; i<QUEUE_SIZE;i++){
        queue->data[i] = (packet*) malloc(sizeof(packet));
    }
}

void Enqueue(Queue *q)
{
    q->size ++;
    q->rear = (q->rear+1)%QUEUE_SIZE;
}

packet* Rear(Queue *q){
    return q->data[q->rear];
}

packet* Dequeue(Queue *q)
{
    q->size -- ;
    packet* tmp = q->data[q->front];
    q->front = (q->front+1)%QUEUE_SIZE;
    return tmp;
}

packet* Front(Queue *q){
    return q->data[q->front];
}

