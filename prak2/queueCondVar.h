#ifndef PRAK2_QUEUECONDVAR_H
#define PRAK2_QUEUECONDVAR_H
#include <pthread.h>
#include <semaphore.h>

struct Node {
    char val;
    struct Node *next;
};

struct NodeTail {
    struct Node *prev;
};

typedef struct Queue {
    struct Node *head;
    struct NodeTail *tail;
    int size;
    int capacity;
    pthread_cond_t cvEmpty;
    pthread_cond_t cvFull;
    pthread_mutex_t mutex;
} Queue;

Queue *initializeQueue(int capacity);

//returns -1 if queue is full, else returns 0
char enqueue(Queue *queue, char val);

//returns -1 if queue is empty, else returns dequeued item
char dequeue(Queue *queue);

int getQueueSize(Queue *queue);

char* queueToString(Queue *queue);

#endif //PRAK2_QUEUECONDVAR_H
