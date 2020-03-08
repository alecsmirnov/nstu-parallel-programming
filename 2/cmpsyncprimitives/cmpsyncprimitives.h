#ifndef CMPSYNCPRIMITIVES_H
#define CMPSYNCPRIMITIVES_H

#include <stdint.h>

// Количество блокировок потока (доступов к общим данным)
#define LOCKS_COUNT 100000000

// Тип используемого примитива
typedef enum SyncPrimitive {
    MUTEX,
    SPINLOCK
} SyncPrimitive;

// Время работы примитива с указанной функцией, кол-вом потоков
double primitiveTimeStat(SyncPrimitive primitive, uint8_t threads_count);

#endif