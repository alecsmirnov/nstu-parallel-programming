#ifndef CONDVARIMITATION_H
#define CONDVARIMITATION_H

#include <stdbool.h>
#include <pthread.h>

#define WAIT_TIME_MS 1e+6

typedef struct CondStateNode {
    bool* state;

    struct CondStateNode* next;
} CondStateNode;

typedef struct CondType {
    pthread_mutex_t mutex;

    struct CondStateNode* state_queue;
} CondType;

void pthreadCondInit(CondType* cond);

void pthreadCondWait(CondType* cond, pthread_mutex_t* mutex);

void pthreadCondSignal(CondType* cond);
void pthreadCondBroadcast(CondType* cond);

void pthreadCondDestroy(CondType* cond);

#endif