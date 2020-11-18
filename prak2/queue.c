//
// Created by hugo on 18.11.20.
//

#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

void enqueue(node **head, char val) {
    node *newNode = malloc(sizeof(node));
    if (!newNode) return;

    newNode->val = val;
    newNode->next = *head;

    *head = newNode;
}


char dequeue(node **head) {
    node *current, *prev = NULL;
    char retval = -1;

    if (*head == NULL) return -1;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}
