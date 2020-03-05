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

typedef struct CondNode {
    bool* state;

    struct CondNode* next;
} CondNode;

struct CondQueue {
    struct CondNode* head;
    struct CondNode* tail;
};

static void condQueuePush(CondQueue* queue, bool* state) {
    CondNode* new_node = (CondNode*)malloc(sizeof(CondNode));

    new_node->state = state;
    new_node->next = NULL;

    if (queue->head) {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    else 
        queue->head = queue->tail = new_node;
}

static bool* condQueuePop(CondQueue* queue) {
    bool* state = NULL;

    if (queue->head) {
        state = queue->head->state;
        CondNode* delete_node = queue->head;

        if (queue->head != queue->tail)
            queue->head = queue->head->next;
        else
            queue->head = queue->tail = NULL;

        free(delete_node);
    }

    return state;
}

void pthreadCondInit(CondType* cond) {
    int err = pthread_mutex_init(&cond->mutex, NULL);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot initialize mutex");

    cond->queue = NULL;
}

void pthreadCondWait(CondType* cond, pthread_mutex_t* mutex) {
    int err = pthread_mutex_lock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock condition mutex");

    bool* lock = (bool*)malloc(sizeof(bool));
    *lock = true;
    condQueuePush(&cond->queue, lock);

    err = pthread_mutex_unlock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock condition mutex");

    err = pthread_mutex_unlock(mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock mutex");

    while (*lock)
        usleep(WAIT_TIME_MS);
    free(lock);

    err = pthread_mutex_lock(mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock mutex");
}

void pthreadCondSignal(CondType* cond) {
    int err = pthread_mutex_lock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock condition mutex");

    bool* lock = condQueuePop(&cond->queue);
    if (lock)
        *lock = false;

    err = pthread_mutex_unlock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock condition mutex");
}

void pthreadCondBroadcast(CondType* cond) {
    int err = pthread_mutex_lock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock condition mutex");

    bool* lock = NULL;
    while ((lock = condQueuePop(&cond->queue))) 
        *lock = false;

    err = pthread_mutex_unlock(&cond->mutex);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock condition mutex");
}

void pthreadCondDestroy(CondType* cond) {
    pthread_mutex_destroy(&cond->mutex);
    while (condQueuePop(&cond->queue));
}