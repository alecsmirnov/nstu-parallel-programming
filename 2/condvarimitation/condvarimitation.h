#ifndef CONDVARIMITATION_H
#define CONDVARIMITATION_H

#include <pthread.h>

#define WAIT_TIME_MS 1E+6

typedef struct CondQueue CondQueue;

typedef struct CondType {
    pthread_mutex_t mutex;

    struct CondQueue* queue;
} CondType;

void pthreadCondInit(CondType* cond);
void pthreadCondDestroy(CondType* cond);

void pthreadCondWait(CondType* cond, pthread_mutex_t* mutex);

void pthreadCondSignal(CondType* cond);
void pthreadCondBroadcast(CondType* cond);


#endif