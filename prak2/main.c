#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "queue.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

typedef struct ThreadsArg {
    pthread_mutex_t producer1Mtx;
    pthread_mutex_t producer2Mtx;
    pthread_mutex_t consumerMtx;
    Queue *queue;
} ThreadsArg;

ThreadsArg *thrArg = 0;

void *thread3(void *threadsArg);

void *thread2(void *threadsArg);

void *thread1(void *threadsArg);

void *controllthread(void *threadsArg);

void printHelp();

int main() {
    Queue *myQueue = initializeQueue(10000);
    thrArg = calloc(1, sizeof(ThreadsArg));

    printHelp();

    pthread_mutex_t mtx1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mtx2 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mtx3 = PTHREAD_MUTEX_INITIALIZER;

    thrArg->queue = myQueue;
    thrArg->producer1Mtx = mtx1;
    thrArg->producer2Mtx = mtx2;
    thrArg->consumerMtx = mtx3;

    pthread_t threadId1;
    pthread_t threadId2;
    pthread_t threadId3;
    pthread_t threadId4;

    int t1 = pthread_create(&threadId1, 0, thread1, &thrArg);
    int t2 = pthread_create(&threadId2, 0, thread2, &thrArg);
    int t3 = pthread_create(&threadId3, 0, thread3, &thrArg);
    int t4 = pthread_create(&threadId4, 0, controllthread, &thrArg);
    pthread_join(threadId1, 0);
    pthread_join(threadId2, 0);
    pthread_join(threadId3, 0);
    pthread_join(threadId4, 0);

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

void printHelp();

void *thread1(void *_) {
    Queue *myQueue = thrArg->queue;
    while (1) {
        for (char i = 'a'; i <= 'z'; ++i) {
            pthread_mutex_lock(&thrArg->producer1Mtx);
            enqueue(myQueue, i);
            int size = getQueueSize(myQueue);
            printf("%c   (%2d)   P1 \r\n", i, size);
            pthread_mutex_unlock(&thrArg->producer1Mtx);
            sleep(1);
        }
    }
}

void *thread2(void *_) {
    Queue *myQueue = thrArg->queue;
    while (1) {
        for (char i = 'A'; i <= 'Z'; ++i) {
            pthread_mutex_lock(&thrArg->producer2Mtx);
            enqueue(myQueue, i);
            int size = getQueueSize(myQueue);
            printf("%c   (%2d)     P2 \r\n", i, size);
            pthread_mutex_unlock(&thrArg->producer2Mtx);
            sleep(1);
        }
    }
}

void *thread3(void *_) {
    Queue *myQueue = thrArg->queue;
    while (1) {
        pthread_mutex_lock(&thrArg->consumerMtx);
        char res = dequeue(myQueue);
        int size = getQueueSize(myQueue);
        printf("  %c (%2d) CS\r\n", res, size);
        pthread_mutex_unlock(&thrArg->consumerMtx);
        sleep(1);
    }
}

void *controllthread(void *threadsArg) {
    char exit = 0;
    char pr1 = 0;
    char pr2 = 0;
    char cs = 0;
    while (!exit) {
        srand(time(0));
        char input = rand() % 'z';
        switch (input) {
            case '1':
                if (pr1) {
                    pthread_mutex_unlock(&thrArg->producer1Mtx);
                    printf(" -- on: ");
                } else {
                    pthread_mutex_lock(&thrArg->producer1Mtx);
                    printf(" -- off: ");
                }
                pr1 = !pr1;
                printf("producer 1\n");
                break;
            case '2':
                if (pr2) {
                    pthread_mutex_unlock(&thrArg->producer2Mtx);
                    printf(" -- on: ");
                } else {
                    pthread_mutex_lock(&thrArg->producer2Mtx);
                    printf(" -- off: ");
                }
                pr2 = !pr2;
                printf("producer 2\n");
                //start/stop prod2
                break;
            case 'c':
            case 'C':
                if (cs) {
                    pthread_mutex_unlock(&thrArg->consumerMtx);
                    printf(" -- on: ");
                } else {
                    pthread_mutex_lock(&thrArg->consumerMtx);
                    printf(" -- off: ");
                }
                cs = !cs;
                printf("consumer\n");
                //start/stop cons
                break;
            case 'q':
            case 'Q':
//                exit = 1;
                break;
            case 'h':
                printHelp();
                //help
                break;
            default:
                break;
        }
        sleep(1);
    }
    pthread_exit(0);
}

void printHelp() {
    printf("┏━━━━━━━━HELP━━━━━━━━┓\n");
    printf("┃  Q q - quit        ┃\n");
    printf("┃  C c - consumer    ┃\n");
    printf("┃  1   - producer 1  ┃\n");
    printf("┃  2   - producer 2  ┃\n");
    printf("┗━━━━━━━━━━━━━━━━━━━━┛\n");
}

#pragma clang diagnostic pop
