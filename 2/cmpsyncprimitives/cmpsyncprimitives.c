
#include "cmpsyncprimitives.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define BILLION 1.0E+9

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Функция вычета разности между временными величинами
#define clocktimeDifference(start, stop)            \
	1.0 * (stop.tv_sec - start.tv_sec) +            \
    1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

// Указатель на функция для работы с примитивом 
typedef int (*primitive_func)();

// Структура для работы с примитивом
typedef struct PrimitiveType {
    primitive_func init_func;       // Ф-ия инициализации
    void* init_arg;                 // Аргументы инициализации
    primitive_func destroy_func;    // Ф-ия освобождения ресурсов

    primitive_func lock_func;       // Ф-ия блокировки
    primitive_func unlock_func;     // Ф-ия разблокировки

    void* type;                     // Примитив
} PrimitiveType;

// Аргументы потока
typedef struct ThreadArg {
    primitive_func lock_func;       // Ф-ия блокировки
    primitive_func unlock_func;     // Ф-ия разблокировки

    void* type;                     // Примитив

    size_t* shared_data;            // Общие данные
} ThreadArg;

// Функция потока, доступа к примитиву
static void* threadFunc(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    int err = 0;

    // Получить доступ к примитиву LOCKS_COUNT раз
    for (size_t i = 0; i != LOCKS_COUNT; ++i) {
        err = thread_arg->lock_func(thread_arg->type);
        if (err != 0)
            throwErr("Error: cannot lock primitive!");

        ++*thread_arg->shared_data;

        err = thread_arg->unlock_func(thread_arg->type);
        if (err != 0)
            throwErr("Error: cannot unlock primitive");
    }

    pthread_exit(NULL);
}

// Инициализация структуры для работы с примитивом
static void primitiveTypeInit(PrimitiveType* primitive_type, SyncPrimitive primitive) {
    switch (primitive) {
        case MUTEX: {
            primitive_type->init_func    = pthread_mutex_init;
            primitive_type->init_arg     = NULL;
            primitive_type->destroy_func = pthread_mutex_destroy;

            primitive_type->lock_func    = pthread_mutex_lock;
            primitive_type->unlock_func  = pthread_mutex_unlock;

            primitive_type->type         = malloc(sizeof(pthread_mutex_t));
            break;
        }
        case SPINLOCK: {
            primitive_type->init_func    = pthread_spin_init;
            primitive_type->init_arg     = (void*)PTHREAD_PROCESS_PRIVATE;
            primitive_type->destroy_func = pthread_spin_destroy;

            primitive_type->lock_func    = pthread_spin_lock;
            primitive_type->unlock_func  = pthread_spin_unlock;

            primitive_type->type         = malloc(sizeof(pthread_spinlock_t));
            break;
        }
    }
}

// Время работы примитива с указанной функцией, кол-вом потоков
double primitiveTimeStat(SyncPrimitive primitive, uint8_t threads_count) {
    PrimitiveType primitive_type;
    int err = 0;

    size_t shared_data = 0;
    
    // Инициализация примитива
    primitiveTypeInit(&primitive_type, primitive);

    err = primitive_type.init_func(primitive_type.type, primitive_type.init_arg);
    if (err != 0)
        throwErr("Error: cannot initialize primitive");

    // Формирование потоков и их аргументов
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * threads_count);
    ThreadArg threads_arg = (ThreadArg){primitive_type.lock_func, 
                                        primitive_type.unlock_func, 
                                        primitive_type.type, &shared_data};

    struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (uint8_t i = 0; i != threads_count; ++i) {
		err = pthread_create(&threads[i], NULL, threadFunc, (void*)&threads_arg);
		if (err != 0)
            throwErr("Error: cannot create a thread");
	}

	for (uint8_t i = 0; i != threads_count; ++i) {
		err = pthread_join(threads[i], NULL);
		if (err != 0)
            throwErr("Error: cannot join a thread");
	}

    clock_gettime(CLOCK_MONOTONIC, &stop);
	double elapsed_time = clocktimeDifference(start, stop);

    primitive_type.destroy_func(primitive_type.type);
    free(primitive_type.type);
	free(threads);

    return elapsed_time;
}