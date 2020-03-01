#ifndef CONDVARIMITATION_H
#define CONDVARIMITATION_H

#include <pthread.h>

// list data structure from: https://github.com/alecsmirnov/doubly-linked-list
#include "list.h"

#define WAIT_TIME 1e+6

typedef struct CondType {
    pthread_mutex_t mutex;

    List* state_list;
} CondType;

void pthreadCondInit(CondType* cond);

void pthreadCondWait(CondType* cond, pthread_mutex_t* mutex);

void pthreadCondSignal(CondType* cond);
void pthreadCondBroadcast(CondType* cond);

void pthreadCondDestroy(CondType* cond);

#endif