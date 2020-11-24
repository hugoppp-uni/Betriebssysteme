#include <stdlib.h>
#include "queueCondVar.h"
#include <semaphore.h>

char enqueue(Queue *queue, char val) {

    struct Node *newNode = malloc(sizeof(struct Node));
    if (!newNode) {
        return -1;
    }

    newNode->val = val;
    newNode->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    while (queue->size >= queue->capacity) {
        pthread_cond_wait(&queue->cvFull,&queue->mutex);
    }
    queue->size++;
    pthread_cond_signal(&queue->cvEmpty);

    struct NodeTail *nodeTail = queue->tail;
    if (!queue->head) {
        queue->head = newNode;
    } else {
        struct Node *lastNode = nodeTail->prev;
        lastNode->next = newNode;
    }
    nodeTail->prev = newNode;

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

Queue *initializeQueue(int capacity) {
    Queue *queue = calloc(1, sizeof(Queue));
    struct NodeTail *tail = calloc(1, sizeof(struct NodeTail));
    if (!queue || !tail)
        return 0;

    queue->tail = tail;
    queue->head = NULL;
    queue->capacity = capacity;
    queue->size = 0;
    pthread_cond_init(&queue->cvFull,NULL);
    pthread_cond_init(&queue->cvEmpty,NULL);
    pthread_mutex_init(&queue->mutex, NULL);
    return queue;
}

char dequeue(Queue *queue) {

    if (!queue /*|| !queue->head*/) {
        return -1;
    }

    pthread_mutex_lock(&queue->mutex);
    while (queue->size <= 0) {
        pthread_cond_wait(&queue->cvEmpty,&queue->mutex);
    }
    queue->size--;
    pthread_cond_signal(&queue->cvFull);

    struct Node *elementToRemove = queue->head;
    if (elementToRemove->next) {
        queue->head = elementToRemove->next;
    } else {
        //elementToRemove is last element in queue
        queue->head = NULL;
        queue->tail->prev = NULL;
    }

    pthread_mutex_unlock(&queue->mutex);

    char returnVal = elementToRemove->val;
    free(elementToRemove);

    return returnVal;
}

int getQueueSize(Queue *queue) {
    int res = 0;
//    sem_getvalue(&queue->semaphore, &res);
    return res;
}

char *queueToString(Queue *queue) {
    if (!queue)
        return 0;

    pthread_mutex_lock(&queue->mutex);
    if (!queue->size) {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    struct Node *it = queue->head;
    char *str = malloc(queue->size + 1);

    for (int i = 0; i < queue->size; ++i) {
        str[i] = it->val;
        it = it->next;
    }
    pthread_mutex_unlock(&queue->mutex);
    str[queue->size] = '\0';
    return str;
}
