#ifndef CMPSYNCPRIMITIVES_H
#define CMPSYNCPRIMITIVES_H

#include <stdlib.h>
#include <stdint.h>

#define LOCKS_COUNT 10000000

typedef void (*test_func)(size_t);

typedef enum SyncPrimitive {
    MUTEX,
    SPINLOCK
} SyncPrimitive;

double primitiveTimeStat(test_func func, SyncPrimitive primitive, uint8_t threads_count);

#endif