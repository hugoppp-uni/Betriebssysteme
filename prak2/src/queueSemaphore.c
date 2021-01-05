#include <stdlib.h>
#include "queueSemaphore.h"
#include <semaphore.h>

char enqueue(Queue *queue, char val) {

    struct Node *newNode = malloc(sizeof(struct Node));
    if (!newNode) {
        return -1;
    }

    newNode->val = val;
    newNode->next = NULL;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
    sem_wait(&queue->semaphoreInverted);
    pthread_mutex_lock(&queue->mutex);
    if (queue->exit) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

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
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    return 0;
}

void exitQueue(Queue *queue) {
    queue->exit = 1;
    for (int i = 0; i <= queue->maxThreads + 1; i++) {
        sem_post(&queue->semaphore);
        sem_post(&queue->semaphoreInverted);
    }
}

Queue *initializeQueue(int capacity, int maxThreads) {
    Queue *queue = calloc(1, sizeof(Queue));
    struct NodeTail *tail = calloc(1, sizeof(struct NodeTail));
    if (!queue || !tail)
        return 0;

    queue->tail = tail;
    queue->head = NULL;
    queue->maxThreads = maxThreads;
    sem_init(&queue->semaphore, 0, 0);
    sem_init(&queue->semaphoreInverted, 0, capacity);
    pthread_mutex_init(&queue->mutex, NULL);
    return queue;
}

char dequeue(Queue *queue) {

    if (!queue /*|| !queue->head*/) {
        return -1;
    }

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
    sem_wait(&queue->semaphore);
    pthread_mutex_lock(&queue->mutex);
    if (queue->exit) {
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

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->semaphoreInverted);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);

    char returnVal = elementToRemove->val;
    free(elementToRemove);

    return returnVal;
}

int getQueueSize(Queue *queue) {
    int res = 0;
    sem_getvalue(&queue->semaphore, &res);
    return res;
}

char *queueToString(Queue *queue) {
    if (!queue)
        return 0;

    int len;
    pthread_mutex_lock(&queue->mutex);
    if (queue->exit){
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    sem_getvalue(&queue->semaphore, &len);
    if (!len) {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    struct Node *it = queue->head;
    char *str = malloc(len + 1);

    for (int i = 0; i < len; ++i) {
        str[i] = it->val;
        it = it->next;
    }
    pthread_mutex_unlock(&queue->mutex);
    str[len] = '\0';
    return str;
}
