//
// Created by hugo on 18.11.20.
//

#ifndef PRAK2_QUEUE_H
#define PRAK2_QUEUE_H
#endif //PRAK2_QUEUE_H

typedef struct node {
    char val;
    struct node *next;
} node;


//returns -1 if buffer is full, else returns 0
void enqueue(node **head, char val);
char dequeue(node **head);
