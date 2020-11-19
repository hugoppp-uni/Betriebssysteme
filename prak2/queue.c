#include <stdlib.h>
#include "queue.h"

char enqueue(Queue *queue, char val) {
    if (!queue || queue->size >= queue->capacity)
        return -1;

    struct Node *newNode = malloc(sizeof(struct Node));
    if (!newNode)
        return -1;
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
    queue->size++;
    return 0;
}

Queue *initializeQueue(int capacity) {
    struct NodeTail *tail = calloc(1, sizeof(struct NodeTail));
    Queue *queue = calloc(1, sizeof(Queue));
    queue->size = 0;
    queue->capacity = capacity;
    queue->tail = tail;
    queue->head = NULL;
    return queue;
}

char dequeue(Queue *queue) {
    if (!queue || !queue->head)
        return -1;

    struct Node *elementToRemove = queue->head;
    if (elementToRemove->next) {
        queue->head = elementToRemove->next;
    } else {
        //elementToRemove is last element in queue
        queue->head = NULL;
        queue->tail->prev = NULL;
    }
    queue->size--;
    char returnVal = elementToRemove->val;
    free(elementToRemove);
    return returnVal;
}
