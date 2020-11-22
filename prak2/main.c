#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "queue.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread3(void *queue);
void *thread2(void *queue);
void *thread1(void *queue);

pthread_cond_t cond1;

int main() {
    Queue *myQueue = initializeQueue(10000);
    pthread_t threadId1;
    pthread_t threadId2;
    pthread_t threadId3;
    int t1 = pthread_create(&threadId1, NULL, thread1, myQueue);
    int t3 = pthread_create(&threadId3, NULL, thread3, myQueue);
    int t2 = pthread_create(&threadId2, NULL, thread2, myQueue);
    pthread_join(threadId1, NULL);
    pthread_join(threadId2, NULL);
    pthread_join(threadId3, NULL);


    t1;
    t2;

    while (1) {
        for (char i = 0; i < 10; ++i) {
            enqueue(myQueue, 'a' + i);
        }
        for (char i = 0; i < 11; ++i) {
            char c = dequeue(myQueue);
            printf("%c\r\n", c);
        }
    }
}

// pass Queue* as parameter

int counter = 0;

void *thread1(void *queue) {
    Queue *myQueue = queue;
    while (1) {
        for (char i = 'a'; i <= 'z'; ++i) {
            sleep(1);
            enqueue(myQueue, i);
            int size = getQueueSize(queue);
            printf(">P1(%d): %c\r\n", size, i);
        }
    }
}

void *thread2(void *queue) {
    Queue *myQueue = queue;
    while (1) {
        for (char i = 'A'; i <= 'Z'; ++i) {
            sleep(1);
            enqueue(myQueue, i);
            int size = getQueueSize(queue);
            printf(">P2(%d): %c\r\n", size, i);
        }
    }
}

void *thread3(void *queue) {
    Queue *myQueue = queue;
    while (1) {
        sleep(1);
        char res = dequeue(myQueue);
        int size = getQueueSize(queue);
        printf("<C1(%d): %c\r\n", size, res);
    }
}


#pragma clang diagnostic pop