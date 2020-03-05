#include "condvarimitation.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define PTHREAD_PASS 0

#define pthreadErrorHandle(test_code, pass_code, err_msg) do {		\
	if (test_code != pass_code) {									\
		fprintf(stderr, "%s: %s\n", err_msg, strerror(test_code));	\
		exit(EXIT_FAILURE);											\
	}																\
} while (0)

struct CondQueue {
    bool* state;

    struct CondQueue* next;
};

static void stateQueuePush(CondQueue** head, bool* state) {
   CondQueue* new_node = (CondQueue*)malloc(sizeof(CondQueue));
   
   new_node->state = state;
   new_node->next = *head;

   *head = new_node;
}

static bool* stateQueuePop(CondQueue** head) {
    bool* state = NULL;

    if (*head) {
        CondQueue* iter = *head;
        CondQueue* prev = NULL;

        while (iter->next) {
            prev = iter;
            iter = iter->next;
        }

        state = iter->state;
        free(iter);

        if (prev)
            prev->next = NULL;
        else
            *head = NULL;
    }

    return state;
}

void pthreadCondInit(CondType* cond) {
    int err = pthread_mutex_init(&cond->mutex, NULL);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot initialize mutex");

    cond->state_queue = NULL;
}

void pthreadCondWait(CondType* cond, pthread_mutex_t* mutex) {
    int err = pthread_mutex_lock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock condition mutex");

    bool* lock = (bool*)malloc(sizeof(bool));
    *lock = true;
    stateQueuePush(&cond->state_queue, lock);

    err = pthread_mutex_unlock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock condition mutex");

    err = pthread_mutex_unlock(mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock mutex");

    while (*lock)
        usleep(WAIT_TIME_MS);

    err = pthread_mutex_lock(mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock mutex");
    free(lock);
}

void pthreadCondSignal(CondType* cond) {
    int err = pthread_mutex_lock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock condition mutex");

    bool* lock = stateQueuePop(&cond->state_queue);
    if (lock)
        *lock = false;

    err = pthread_mutex_unlock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock condition mutex");
}

void pthreadCondBroadcast(CondType* cond) {
    int err = pthread_mutex_lock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock condition mutex");

    bool* lock = NULL;
    while ((lock = stateQueuePop(&cond->state_queue))) 
        *lock = false;

    err = pthread_mutex_unlock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock condition mutex");
}

void pthreadCondDestroy(CondType* cond) {
    pthread_mutex_destroy(&cond->mutex);
    while (stateQueuePop(&cond->state_queue));
}