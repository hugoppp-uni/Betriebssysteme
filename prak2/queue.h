#ifndef PRAK2_QUEUE_H
#define PRAK2_QUEUE_H

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
} Queue;

Queue *initializeQueue(int capacity);

//returns -1 if queue is full, else returns 0
char enqueue(Queue *queue, char val);

//returns -1 if queue is empty, else returns dequeued item
char dequeue(Queue *queue);

#endif //PRAK2_QUEUE_H
