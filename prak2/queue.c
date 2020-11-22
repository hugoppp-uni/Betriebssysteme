#include <stdlib.h>
#include "queue.h"
#include <semaphore.h>

char enqueue(Queue *queue, char val) {

    sem_wait(&queue->semaphoreInverted);
    pthread_mutex_lock(&queue->mutex);

    struct Node *newNode = malloc(sizeof(struct Node));
    if (!newNode) {
        sem_post(&queue->semaphoreInverted);
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    newNode->val = val;
    newNode->next = NULL;

    struct NodeTail *nodeTail = queue->tail;
    if (!queue->head) {
        queue->head = newNode;
    } else {
        struct Node *lastNode = nodeTail->prev;
        lastNode->next = newNode;
    }
    nodeTail->prev = newNode;

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->semaphore);
    return 0;
}

Queue *initializeQueue(int capacity) {
    Queue *queue = calloc(1, sizeof(Queue));
    struct NodeTail *tail = calloc(1, sizeof(struct NodeTail));
    queue->tail = tail;
    queue->head = NULL;
    sem_init(&queue->semaphore, 0, 0);
    sem_init(&queue->semaphoreInverted, 0, capacity);
    pthread_mutex_init(&queue->mutex, NULL);
    return queue;
}

char dequeue(Queue *queue) {
    sem_wait(&queue->semaphore);
    pthread_mutex_unlock(&queue->mutex);

    if (!queue || !queue->head) {
        sem_post(&queue->semaphore);
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    struct Node *elementToRemove = queue->head;
    if (elementToRemove->next) {
        queue->head = elementToRemove->next;
    } else {
        //elementToRemove is last element in queue
        queue->head = NULL;
        queue->tail->prev = NULL;
    }
    char returnVal = elementToRemove->val;
    free(elementToRemove);

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->semaphoreInverted);

    return returnVal;
}

int getQueueSize(Queue *queue) {
    int res = 0;
    sem_getvalue(&queue->semaphore, &res);
    return res;
}
