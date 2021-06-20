// Idan Givati 315902239
#include <stdio.h>
#include <stdlib.h>
#include "threadPool.h"

ThreadPool* tpCreate(int numOfThreads) {
    // if we
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
        pthread_create(&tp->threads[i], NULL, NULL, NULL);
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
