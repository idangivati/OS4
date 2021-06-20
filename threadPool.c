// Idan Givati 315902239
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "threadPool.h"

void checkForFunc(ThreadPool tp) {

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
    }
    for (int i = 0; i < numOfThreads; i ++) {
        ret = pthread_create(&tp->threads[i], NULL, (void *) checkForFunc, tp);
        if( ret != 0 )
        {
            perror("pthread failed");
            for (i; i >= 0; i--) {
                free((void *) tp->threads[i]);
            }
            exit(0);
        }
    }
    tp->numThreads = numOfThreads;
    return tp;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks){
    // destroying the thread pool.
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    // inserting task.
}
