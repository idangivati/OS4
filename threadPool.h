#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include <pthread.h>
#include <stdbool.h>

typedef void (*function)(void *arg);
typedef struct thread_params {
    function func;
    void* args;
} ThreadParams;

typedef struct thread_pool
{
    OSQueue *funcQ;
    int stop;
    pthread_t* threads;
    pthread_mutex_t task_mutex;
    int num_of_threads;
    int working_threads;
    int need_to_wait;
    pthread_cond_t   task_cond;
    pthread_cond_t   tasking_cond;
    int num_of_tasks;
} ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
