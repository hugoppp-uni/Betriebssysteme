#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "queue.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main() {
    Queue *myQueue = initializeQueue(10);

    while (1) {
        for (char i = 0; i < 10; ++i) {
            enqueue(myQueue, 'a' + i);
        }
        for (char i = 0; i < 11; ++i) {
            char c = dequeue(myQueue);
            printf("%c\r\n", c);
        }
    }

#pragma clang diagnostic pop
}
