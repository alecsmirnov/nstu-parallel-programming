#ifndef CMPSYNCPRIMITIVES_H
#define CMPSYNCPRIMITIVES_H

#include <stdlib.h>
#include <stdint.h>

#define LOCKS_COUNT 10000000

typedef enum SyncPrimitive {
    MUTEX,
    SPINLOCK
} SyncPrimitive;

double primitiveTimeStat(SyncPrimitive primitive, uint8_t threads_count);

#endif