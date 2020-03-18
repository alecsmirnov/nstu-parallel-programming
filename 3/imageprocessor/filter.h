#ifndef FILTER_H
#define FILTER_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define filterCreate(filter, ...) do {                                      \
    static double filter_const[] = __VA_ARGS__;                             \
    static uint16_t filter_size = sizeof(filter_const) / sizeof(double);    \
    filter.data = malloc(filter_size * sizeof(double));                     \
    if (filter.data == NULL)                                                \
        fprintf(stderr, "Error: filter out of memmory!\n");                 \
    memcpy(filter.data, filter_const, filter_size * sizeof(double));        \
    filter.r = sqrt(filter_size);                                           \
} while (0)

#define filterAccess(filter, i, j) \
    (filter.data[(i) * (filter.r) + (j)])

typedef struct Filter {
    double* data;
    uint8_t r;

    double factor;
    double bias;
} Filter;

#endif