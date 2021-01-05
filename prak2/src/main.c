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

#define PROD_COUNT 2

typedef struct ThreadsArg {
    Queue *queue;
    //once locked, these mutexes will switch of the respective thread
    pthread_mutex_t *producers[PROD_COUNT];
    pthread_mutex_t *consumerMtx;
} ThreadsArg;

pthread_mutex_t consoleMtx;
typedef struct ConsProdArg {
    Queue *queue;
    pthread_mutex_t controllMutex;
    char isUppercase;
} ConsProdArg;

void *consumer(void *threadsArg);

void *producer(void *threadsArg);

void *controllThread(void *threadsArg);

void printHelp();

int main() {
#ifdef USE_COND_VARS
    printf("using condvars\n");
#else
    printf("using semaphore\n");
#endif

    Queue *myQueue = initializeQueue(10, 3);
    ThreadsArg *thrArg = calloc(1, sizeof(ThreadsArg));
    pthread_mutex_init(&consoleMtx, NULL);
    thrArg->queue = myQueue;

    ConsProdArg cons = {.queue = myQueue, .isUppercase = 0};
    pthread_mutex_init(&cons.controllMutex, NULL);

    ConsProdArg prod[PROD_COUNT] = {
        {.queue = myQueue, .isUppercase = 1},
        {.queue = myQueue, .isUppercase = 0},
    };
    for (int i = 0; i < PROD_COUNT; ++i) {
        pthread_mutex_init(&prod[i].controllMutex, NULL);
        thrArg->producers[i] = &prod[i].controllMutex;
    }
    thrArg->consumerMtx = &cons.controllMutex;

#define THREAD_COUNT 4
    pthread_t threadId[THREAD_COUNT] = {-1};
    int thrErrs[THREAD_COUNT] = {-1};

    thrErrs[0] = pthread_create(&threadId[0], 0, consumer, &cons);
    thrErrs[1] = pthread_create(&threadId[1], 0, controllThread, thrArg);
    int prodOfset = THREAD_COUNT - PROD_COUNT;
    for (int i = 0; i < PROD_COUNT; ++i) {
        thrErrs[prodOfset] = pthread_create(&threadId[i + prodOfset], 0, producer, &prod[i]);
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
        printf("Thread with id %lu, returned error code %d\n", threadId[i], thrErrs[i]);
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threadId[i], 0);
    }


#define EXIT_VALS_SIZE 7
    int exitVals[EXIT_VALS_SIZE];
    exitVals[0] = pthread_mutex_destroy(thrArg->producers[0]);
    exitVals[1] = pthread_mutex_destroy(thrArg->producers[1]);
    exitVals[2] = pthread_mutex_destroy(thrArg->consumerMtx);
    exitVals[3] = pthread_mutex_destroy(&consoleMtx);
    exitVals[4] = pthread_mutex_destroy(&myQueue->mutex);
#ifdef USE_COND_VARS
    exitVals[5] = pthread_cond_destroy(&myQueue->cvEmpty);
    exitVals[6] = pthread_cond_destroy(&myQueue->cvEmpty);
#else
    exitVals[5] = sem_destroy(&myQueue->semaphore);
    exitVals[6] = sem_destroy(&myQueue->semaphoreInverted);
#endif
    printf("\nmutex and sem / condVar return values:\n");
    printf("-- Controll Thread: --\n");
    for (int i = 0; i < EXIT_VALS_SIZE; ++i) {
        if (i == 4)
            printf("-- Queue Threads: --\n");
        printf("index %d: %d\n", i, exitVals[i]);
    }
    free(thrArg);
    // there should be a method in queue.h that frees all stack memory that the queue uses, but ¯\_(ツ)_/¯
    if (myQueue->tail)
        free(myQueue->tail);
    free(myQueue);
}


void *producer(void *arg) {
    ConsProdArg *thrArg = (ConsProdArg *) arg;
    Queue *myQueue = thrArg->queue;
    while (1) {
        char startAt = thrArg->isUppercase ? 'A' : 'a';
        char endAt = thrArg->isUppercase ? 'Z' : 'z';
        for (char i = startAt; i <= endAt; ++i) {

            pthread_mutex_lock(&thrArg->controllMutex);
            pthread_mutex_unlock(&thrArg->controllMutex);
            enqueue(myQueue, i);
            if (thrArg->queue->exit) {
                pthread_exit(0);
            }

            char *s = queueToString(myQueue);
            printf("     |%-10s| <- %c\n", s, i);
            if (s)
                free(s);

            sleep(1);
        }
    }
}


void *consumer(void *arg) {
    ConsProdArg *thrArg = (ConsProdArg *) arg;
    Queue *myQueue = thrArg->queue;
    while (1) {
        pthread_mutex_lock(&thrArg->controllMutex);
        pthread_mutex_unlock(&thrArg->controllMutex);
        char res = dequeue(myQueue);
        if (thrArg->queue->exit) {
            pthread_exit(0);
        }

        char *s = queueToString(myQueue);
        printf("%c <- |%-10s| \n", res, s);
        if (s)
            free(s);

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
                    pthread_mutex_lock(thrArg->producers[0]);
                    printf(" -- off: producer 1 --\n");
                } else {
                    pthread_mutex_unlock(thrArg->producers[0]);
                    printf(" -- on: producer 1 --\n");
                }
                pr1On = !pr1On;
                break;
            case '2':
                if (pr2On) {
                    pthread_mutex_lock(thrArg->producers[1]);
                    printf(" -- off: producer 2 --\n");
                } else {
                    pthread_mutex_unlock(thrArg->producers[1]);
                    printf(" -- on: producer 2 --\n");
                }
                pr2On = !pr2On;
                break;
            case 'c':
            case 'C':
                if (cOn) {
                    pthread_mutex_lock(thrArg->consumerMtx);
                    printf(" -- off: consumer --\n");
                } else {
                    pthread_mutex_unlock(thrArg->consumerMtx);
                    printf(" -- on: consumer --\n");
                }
                cOn = !cOn;
                break;
            case 'q':
            case 'Q': {
                int i1 = 0, i2 = 0, i3 = 0;
                thrArg->queue->exit = 1;
                if (!pr1On)
                    i1 = pthread_mutex_unlock(thrArg->producers[0]);
                if (!pr2On)
                    i2 = pthread_mutex_unlock(thrArg->producers[1]);
                if (!cOn)
                    i3 = pthread_mutex_unlock(thrArg->consumerMtx);
                printf("Exiting. Errors: %d,%d,%d\n", i1, i2, i3);
                exitQueue(thrArg->queue);
                break;
            }
            case 'h':
                printHelp();
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
