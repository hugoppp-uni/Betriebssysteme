#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


#ifdef USE_COND_VARS

#include "queueCondVar.h"

#else

#include "queueSemaphore.h"

#endif

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

#define THR_SAFE_CONSOLE(s) pthread_mutex_lock(&thrArg->consoleMtx); \
                    s;                                            \
                    pthread_mutex_unlock(&thrArg->consoleMtx);

typedef struct ThreadsArg {
    Queue *queue;
    //once locked, these mutexes will switch of the respective thread
    pthread_mutex_t producer1Mtx;
    pthread_mutex_t producer2Mtx;
    pthread_mutex_t consumerMtx;
    pthread_mutex_t consoleMtx;
} ThreadsArg;

typedef struct ConsProdArg {
    Queue *queue;
    pthread_mutex_t mutex;
    char isUppercase;
} ConsProdArg;

void *consumer(void *threadsArg);

void *producer2(void *threadsArg);

void *producer1(void *threadsArg);

void *controllThread(void *threadsArg);

void printHelp();

int main() {
    Queue *myQueue = initializeQueue(10, 3);
    ThreadsArg *thrArg = calloc(1, sizeof(ThreadsArg));

    pthread_mutex_init(&thrArg->producer1Mtx, NULL);
    pthread_mutex_init(&thrArg->producer2Mtx, NULL);
    pthread_mutex_init(&thrArg->consumerMtx, NULL);
    pthread_mutex_init(&thrArg->consoleMtx, NULL);

    thrArg->queue = myQueue;

    pthread_t threadId[4];
    int t1 = pthread_create(&threadId[0], 0, producer1, thrArg);
    int t2 = pthread_create(&threadId[1], 0, producer2, thrArg);
    int t3 = pthread_create(&threadId[2], 0, consumer, thrArg);
    int t4 = pthread_create(&threadId[3], 0, controllThread, thrArg);

    for (int i = 0; i < 4; i++) {
        pthread_join(threadId[i], 0);
    }


#define EXIT_VALS_SIZE 7
    int exitVals[EXIT_VALS_SIZE];
    exitVals[0] = pthread_mutex_destroy(&myQueue->mutex);
    exitVals[1] = pthread_mutex_destroy(&thrArg->producer1Mtx);
    exitVals[2] = pthread_mutex_destroy(&thrArg->producer2Mtx);
    exitVals[3] = pthread_mutex_destroy(&thrArg->consumerMtx);
    exitVals[4] = pthread_mutex_destroy(&thrArg->consoleMtx);
#ifdef USE_COND_VARS
    exitVals[5] = pthread_cond_destroy(&myQueue->cvEmpty);
    exitVals[6] = pthread_cond_destroy(&myQueue->cvEmpty);
#else
    exitVals[5] = sem_destroy(&myQueue->semaphore);
    exitVals[6] = sem_destroy(&myQueue->semaphoreInverted);
#endif
    printf("mutex and sem / condVar val:\n");
    for (int i = 0; i < EXIT_VALS_SIZE; i++) {
        printf("index %d: %d\n", i, exitVals[i]);
    }
}


void *producer1(void *arg) {
    ThreadsArg *thrArg = (ThreadsArg *) arg;
    Queue *myQueue = thrArg->queue;
    while (1) {
        for (char i = 'a'; i <= 'z'; ++i) {
            pthread_mutex_lock(&thrArg->producer1Mtx);
            enqueue(myQueue, i);

            char *s = queueToString(myQueue);
            printf("     |%-10s| <- %c\n", s, i);

            pthread_mutex_unlock(&thrArg->producer1Mtx);
            sleep(1);
        }
    }
}

void *producer2(void *arg) {
    ThreadsArg *thrArg = (ThreadsArg *) arg;
    Queue *myQueue = thrArg->queue;
    while (1) {
        for (char i = 'A'; i <= 'Z'; ++i) {
            pthread_mutex_lock(&thrArg->producer2Mtx);
            enqueue(myQueue, i);

            char *s = queueToString(myQueue);
            THR_SAFE_CONSOLE(printf("     |%-10s| <- %c\n", s, i));

            pthread_mutex_unlock(&thrArg->producer2Mtx);
            sleep(1);
        }
    }
}

void *consumer(void *arg) {
    ThreadsArg *thrArg = (ThreadsArg *) arg;
    Queue *myQueue = thrArg->queue;
    while (1) {
        pthread_mutex_lock(&thrArg->consumerMtx);
        char res = dequeue(myQueue);

        char *s = queueToString(myQueue);
        THR_SAFE_CONSOLE(printf("%c <- |%-10s| \n", res, s));

        pthread_mutex_unlock(&thrArg->consumerMtx);
        sleep(1);
    }
}

void *controllThread(void *arg) {
    ThreadsArg *thrArg = (ThreadsArg *) arg;
    char pr1On = 1;
    char pr2On = 1;
    char cOn = 1;
    while (!thrArg->queue->exit) {
        char input = (char) getchar();
        switch (input) {
            case '1':
                if (pr1On) {
                    pthread_mutex_lock(&thrArg->producer1Mtx);
                    THR_SAFE_CONSOLE(printf(" -- off: producer 1 --\n"));
                } else {
                    pthread_mutex_unlock(&thrArg->producer1Mtx);
                    THR_SAFE_CONSOLE(printf(" -- on: producer 1 --\n"));
                }
                pr1On = !pr1On;
                break;
            case '2':
                if (pr2On) {
                    pthread_mutex_lock(&thrArg->producer2Mtx);
                    THR_SAFE_CONSOLE(printf(" -- off: producer 2 --\n"));
                } else {
                    pthread_mutex_unlock(&thrArg->producer2Mtx);
                    THR_SAFE_CONSOLE(printf(" -- on: producer 2 --\n"));
                }
                pr2On = !pr2On;
                break;
            case 'c':
            case 'C':
                if (cOn) {
                    pthread_mutex_lock(&thrArg->consumerMtx);
                    THR_SAFE_CONSOLE(printf(" -- off: consumer --\n"));
                } else {
                    pthread_mutex_unlock(&thrArg->consumerMtx);
                    THR_SAFE_CONSOLE(printf(" -- on: consumer --\n"));
                }
                cOn = !cOn;
                break;
            case 'q':
            case 'Q':
                pthread_mutex_unlock(&thrArg->producer1Mtx);
                pthread_mutex_unlock(&thrArg->producer2Mtx);
                pthread_mutex_unlock(&thrArg->consumerMtx);
                exitQueue(thrArg->queue);
                break;
            case 'h':
            THR_SAFE_CONSOLE(printHelp());
                break;
            default:
                break;
        }
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
