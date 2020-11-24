#ifndef PRAK2_QUEUEMERGED_H
#define PRAK2_QUEUEMERGED_H
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
    pthread_mutex_t mutex;
#ifdef USE_COND_VARS
    int size;
    int capacity;
    pthread_cond_t cvEmpty;
    pthread_cond_t cvFull;
#endif
#ifdef USE_SEMAPHORE
    sem_t semaphore;
    sem_t semaphoreInverted;
#endif
} Queue;

Queue *initializeQueue(int capacity);

//returns -1 if queue is full, else returns 0
char enqueue(Queue *queue, char val);

//returns -1 if queue is empty, else returns dequeued item
char dequeue(Queue *queue);

int getQueueSize(Queue *queue);

char* queueToString(Queue *queue);

#endif //PRAK2_QUEUEMERGED_H