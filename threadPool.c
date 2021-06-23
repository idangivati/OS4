// Idan Givati 315902239
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "threadPool.h"

static void *Tasks(void *arg) {
    ThreadPool *tp = arg;
    while (1) {
        pthread_mutex_lock(&(tp->task_mutex));

        while (tp->num_of_tasks == 0 && !tp->stop)
            pthread_cond_wait(&(tp->task_cond), &(tp->task_mutex));
        if (tp->stop)
            break;

        ThreadParams *tpQ = osDequeue(tp->funcQ);
        tp->num_of_tasks--;
        tp->working_threads++;
        pthread_mutex_unlock(&(tp->task_mutex));

        if (tpQ != NULL) {
            tpQ->func(tpQ->args);
            free(tpQ);
        }

        pthread_mutex_lock(&(tp->task_mutex));
        tp->working_threads--;
        if (!tp->stop && tp->working_threads == 0 && tp->num_of_tasks == 0)
            pthread_cond_signal(&(tp->tasking_cond));
        pthread_mutex_unlock(&(tp->task_mutex));
    }

    tp->num_of_threads--;
    pthread_cond_signal(&(tp->tasking_cond));
    pthread_mutex_unlock(&(tp->task_mutex));
    return NULL;

}
ThreadPool* tpCreate(int numOfThreads) {
    int ret;
    // if we received non positive number.
    if(numOfThreads < 1) {
        return NULL;
    }
    ThreadPool* tp = (ThreadPool *) malloc(sizeof(ThreadPool));
    if( tp == NULL )
    {
        // Malloc failed
        perror("Malloc failed");
        exit(-1);
    }
    pthread_mutex_init(&(tp->task_mutex), NULL);
    pthread_cond_init(&(tp->task_cond), NULL);
    pthread_cond_init(&(tp->tasking_cond), NULL);
    tp->num_of_threads = numOfThreads;
    for (int i = 0; i < numOfThreads; i ++) {
        ret = pthread_create(&tp->threads[i], NULL, (void *) Tasks, tp);
        if( ret != 0 )
        {
            perror("pthread failed");
            for (i; i >= 0; i--) {
                free((void *) tp->threads[i]);
            }
            exit(0);
        }
        pthread_detach(tp->threads[i]);
    }
    return tp;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks){
    // destroying the thread pool.
    if (threadPool == NULL)
        return;
    pthread_mutex_lock(&(threadPool->task_mutex));
    if(threadPool->num_of_tasks != 0) {
        ThreadParams *task = osDequeue(threadPool->funcQ);
        threadPool->num_of_tasks--;
        while (threadPool->num_of_tasks != 0) {
            free(task);
            task = osDequeue(threadPool->funcQ);
        }
    }
    threadPool->stop = true;
    pthread_cond_broadcast(&(threadPool->task_cond));
    pthread_mutex_unlock(&(threadPool->task_mutex));

    pthread_mutex_lock(&(threadPool->task_mutex));
    while (1) {
        if ((!threadPool->stop && threadPool->working_threads != 0) ||
        (threadPool->stop && threadPool->num_of_threads != 0)) {
            pthread_cond_wait(&(threadPool->tasking_cond), &(threadPool->task_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(threadPool->task_mutex));

    pthread_mutex_destroy(&(threadPool->task_mutex));
    pthread_cond_destroy(&(threadPool->task_cond));
    pthread_cond_destroy(&(threadPool->tasking_cond));

    free(threadPool);
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    // inserting task to the queue.
    if(threadPool == NULL) {
        return -1;
    }
    if(computeFunc == NULL) {
        return 0;
    }
    ThreadParams * tParams = (ThreadParams *) malloc(sizeof(ThreadParams));
    if( tParams == NULL )
    {
        // Malloc failed
        perror("Malloc failed");
        return -1;
    }
    tParams->func = computeFunc;
    tParams->args = param;
    osEnqueue(threadPool->funcQ, tParams);
    threadPool->num_of_tasks++;
    return 0;
}
