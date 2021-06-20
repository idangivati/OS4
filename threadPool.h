#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include <pthread.h>

typedef struct thObj {
    void* func;
    void* args;
} thObj;

typedef struct thread_pool
{
    void *thObj;
    pthread_t* threads;
    pthread_mutex_t mutex;
    int numThreads;
    int active;
    int numOfFunc;
    int needToWait;
    int destroyed;
} ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
