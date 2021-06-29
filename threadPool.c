// Idan Givati 315902239
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "threadPool.h"
#include <unistd.h>

/*******************************************************************************
 * This function give a task to each thread if there is any tasks in the queue.
 *******************************************************************************/
static void Tasks(void *arg) {
    ThreadPool *tp = (ThreadPool *) arg;    //the thread pool we working on.
    while (1) {
        if(pthread_mutex_lock(&(tp->task_mutex)) != 0) {
            perror("mutex lock failed");
        }
        while (tp->num_of_tasks == 0 && tp->stop == 0) {
            // if we dont have any task and we didnt called tpDestroy
            pthread_cond_wait(&(tp->task_cond), &(tp->task_mutex));
        }
        if ((tp->stop && tp->need_to_wait ==0) ||
        (tp->stop && tp->need_to_wait && osIsQueueEmpty(tp->funcQ)))
            break;
        ThreadParams *tpQ = (ThreadParams *) osDequeue(tp->funcQ);
        tp->num_of_tasks--;
        tp->working_threads++;
        pthread_mutex_unlock(&(tp->task_mutex));
        if (tpQ != NULL) {
            tpQ->func(tpQ->args);
            free(tpQ);
        }
        pthread_mutex_lock(&(tp->task_mutex));
        tp->working_threads--;
        pthread_mutex_unlock(&(tp->task_mutex));
    }
    tp->num_of_threads--;
    pthread_cond_signal(&(tp->tasking_cond));
    pthread_mutex_unlock(&(tp->task_mutex));
}
/*****************************************************************************
 * Creating a thread pool according to the number of thread requested.
 * @param numOfThreads the number of threads we want to create in the pool.
 * @return the thread pool we created.
 *****************************************************************************/
ThreadPool* tpCreate(int numOfThreads) {
    int ret;
    // if we received non positive number.
    if(numOfThreads < 1) {
        return NULL;
    }
    // from now on, we will create each variable and check if it failed.
    ThreadPool* tp = (ThreadPool *) malloc(sizeof(ThreadPool));
    if( tp == NULL )
    {
        // Malloc failed
        perror("Malloc failed");
        exit(-1);
    }
    tp->stop = 0;   //this indicate if we need to stop or not mainly because of tpDestroy called.
    tp->need_to_wait = 0;
    if((tp->funcQ = osCreateQueue()) == NULL){
        perror("Create Queue failed");
        free(tp);
        exit(-1);
    }
    tp->num_of_threads = numOfThreads;
    tp->threads = (pthread_t *) malloc(sizeof(pthread_t) * tp->num_of_threads);
    if( tp->threads == NULL )
    {
        // Malloc failed
        perror("create threads failed");
        osDestroyQueue(tp->funcQ);
        free(tp);
        exit(-1);
    }
    if(pthread_mutex_init(&(tp->task_mutex), NULL) != 0
    || pthread_cond_init(&(tp->task_cond), NULL) != 0
    || pthread_cond_init(&(tp->tasking_cond), NULL) != 0) {
        perror("initialize mutex failed");
        osDestroyQueue(tp->funcQ);
        free(tp->threads);
        free(tp);
        exit(-1);
    }
    int i;
    for (i = 0; i < numOfThreads; i ++) {
        ret = pthread_create(&tp->threads[i], NULL, (void *)Tasks, (void *)tp);
        if( ret != 0 )
        {
            perror("pthread failed");
            for (i; i >= 0; i--) {
                free((void *) tp->threads[i]);
            }
            osDestroyQueue(tp->funcQ);
            free(tp->threads);
            free(tp);
            exit(-1);
        }
        pthread_detach(tp->threads[i]);
    }
    return tp;
}
/*****************************************************************************************
 * Destroying the thread pool and free all the pointers.
 * @param threadPool the thread pool we want to destroy.
 * @param shouldWaitForTasks indicate if we should wait for tasks. If its 0 we dont wait.
 *****************************************************************************************/
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks){
    // destroying the thread pool.
    if (threadPool == NULL)
        return;
    pthread_mutex_lock(&(threadPool->task_mutex));
    if(threadPool->stop == 1) {
        pthread_mutex_unlock(&(threadPool->task_mutex));
        return;
    }
    threadPool->need_to_wait = shouldWaitForTasks;
    threadPool->stop = 1;
    if(shouldWaitForTasks == 0) {   //destroying the queue.
        while (threadPool->num_of_tasks != 0) {
            ThreadParams *task = osDequeue(threadPool->funcQ);
            threadPool->num_of_tasks--;
            free(task);
        }
    }
    //threadPool->num_of_tasks = 0;
    pthread_cond_broadcast(&(threadPool->task_cond));
    while (1) {
        if (threadPool->num_of_threads > 0) {
            pthread_cond_wait(&(threadPool->tasking_cond), &(threadPool->task_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(threadPool->task_mutex));
    pthread_mutex_destroy(&(threadPool->task_mutex));
    pthread_cond_destroy(&(threadPool->task_cond));
    pthread_cond_destroy(&(threadPool->tasking_cond));
    osDestroyQueue(threadPool->funcQ);
    free(threadPool->threads);
    free(threadPool);
}
/**************************************************************************************
 * Inserting a new task into the queue.
 * @param threadPool the thread pool we that we want it to work on the queue of tasks.
 * @param computeFunc the function.
 * @param param the args.
 * @return 0 if succeeded, -1 if failed.
 **************************************************************************************/
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    // inserting task to the queue.
    if(threadPool == NULL) {
        return -1;
    }
    if(threadPool->stop == 1) {
        return -1;
    }
    if(computeFunc == NULL) {
        return -1;
    }
    if (pthread_mutex_lock(&(threadPool->task_mutex))) {
        perror("Error in lock");
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
    if(pthread_cond_signal(&(threadPool->task_cond)) != 0) {
        perror("fuck you");
    }
    if (pthread_mutex_unlock(&(threadPool->task_mutex))) {
        perror("Error in unlock");
    }
    return 0;
}
