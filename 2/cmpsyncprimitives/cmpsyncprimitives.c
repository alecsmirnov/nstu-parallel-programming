#include "cmpsyncprimitives.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "list.h"

#define BILLION 1.0E+9

#define PTHREAD_PASS 0

#define pthreadErrorHandle(test_code, pass_code, err_msg) do {		\
	if (test_code != pass_code) {									\
		fprintf(stderr, "%s: %s\n", err_msg, strerror(test_code));	\
		exit(EXIT_FAILURE);											\
	}																\
} while (0)

#define clocktimeDifference(start, stop) \
	1.0 * (stop.tv_sec - start.tv_sec) + 1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

typedef int (*primitive_func)();

typedef struct PrimitiveType {
    primitive_func init_func;
    void* init_arg;
    primitive_func destroy_func;

    primitive_func lock_func;
    primitive_func unlock_func;

    void* type;
} PrimitiveType;

typedef struct ThreadArg {
    primitive_func lock_func;
    primitive_func unlock_func;
    void* type;

    List* int_list;
} ThreadArg;

static void* threadFunc(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;

    while (!listIsEmpty(thread_arg->int_list)) {
        int err = thread_arg->lock_func(thread_arg->type);
        pthreadErrorHandle(err, PTHREAD_PASS, "Cannot lock mutex");

        size_t val = *(int*)listBack(thread_arg->int_list);
        listPopBack(thread_arg->int_list);

        err = thread_arg->unlock_func(thread_arg->type);
        pthreadErrorHandle(err, PTHREAD_PASS, "Cannot unlock mutex");
    }

    pthread_exit(NULL);
}

double primitiveTimeStat(SyncPrimitive primitive, uint8_t threads_count) {
    PrimitiveType primitive_type;

    pthread_mutex_t mutex; 
    pthread_spinlock_t spin;

    switch (primitive) {
        case MUTEX: {
            primitive_type.init_func    = pthread_mutex_init;
            primitive_type.init_arg     = NULL;
            primitive_type.destroy_func = pthread_mutex_destroy;

            primitive_type.lock_func    = pthread_mutex_lock;
            primitive_type.unlock_func  = pthread_mutex_unlock;

            primitive_type.type         = (void*)&mutex;
            break;
        }
        case SPINLOCK: {
            primitive_type.init_func    = pthread_spin_init;
            primitive_type.init_arg     = (void*)PTHREAD_PROCESS_PRIVATE;
            primitive_type.destroy_func = pthread_spin_destroy;

            primitive_type.lock_func    = pthread_spin_lock;
            primitive_type.unlock_func  = pthread_spin_unlock;

            primitive_type.type         = (void*)&spin;
            break;
        }
    }

    int err = primitive_type.init_func(primitive_type.type, primitive_type.init_arg);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot initialize primitive");

    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * threads_count);
    ThreadArg threads_arg = (ThreadArg){primitive_type.lock_func, primitive_type.unlock_func, primitive_type.type, NULL};

    listInit(&threads_arg.int_list, sizeof(size_t), NULL);
    for (size_t i = 0; i != LOCKS_COUNT; ++i) 
        listPushBack(threads_arg.int_list, (void*)&i);

    struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

    for (uint8_t i = 0; i != threads_count; ++i) {
		err = pthread_create(&threads[i], NULL, threadFunc, (void*)&threads_arg);
		pthreadErrorHandle(err, PTHREAD_PASS, "Cannot create a thread");
	}

	for (uint8_t i = 0; i != threads_count; ++i) {
		err = pthread_join(threads[i], NULL);
		pthreadErrorHandle(err, PTHREAD_PASS, "Cannot join a thread");
	}

    clock_gettime(CLOCK_MONOTONIC, &stop);
	double elapsed_time = clocktimeDifference(start, stop);

    primitive_type.destroy_func(primitive_type.type);
	free(threads);

    return elapsed_time;
}