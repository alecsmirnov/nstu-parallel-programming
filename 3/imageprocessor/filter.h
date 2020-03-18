#ifndef FILTER_H
#define FILTER_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define filterCreate(filter, factor_init, bias_init, ...) do {  \
    static const double const_data[] = __VA_ARGS__;             \
    filter.data = (double[]) __VA_ARGS__;                       \
    filter.r = sqrt(sizeof(const_data) / sizeof(double));       \
    filter.factor = factor_init;                                \
    filter.bias = bias_init;                                    \
} while (0)

#define filterAccess(filter, i, j) \
    (filter.data[(i) * (filter.r) + (j)])

typedef struct FilterColor {
    double r;
    double g;
    double b;
} FilterColor;

typedef struct Filter {
    double* data;
    uint8_t r;

    double factor;
    double bias;
} Filter;

#endif