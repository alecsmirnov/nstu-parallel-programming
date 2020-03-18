#ifndef FILTER_H
#define FILTER_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// Создание прямоугольного сглаживающего фильтра (const double[])
// Входные аргументы: коэффициент сглаживания, смещение, матрица фильтра
#define filterCreate(factor_init, bias_init, ...) ({        \
    Filter filter;                                          \
    static const double const_data[] = __VA_ARGS__;         \
    filter.matrix = (double[]) __VA_ARGS__;                 \
    filter.r = sqrt(sizeof(const_data) / sizeof(double));   \
    filter.factor = factor_init;                            \
    filter.bias = bias_init;                                \
    filter;                                                 \
})

// Доступ к элементу квадратной матрицы фильтра
#define filterAccess(filter, i, j) \
    ((filter)->matrix[(i) * (filter)->r + (j)])

// Цвет ячейки фильтра
typedef struct FilterColor {
    double r;
    double g;
    double b;
} FilterColor;

typedef struct Filter {
    double* matrix;         // Квадратная матрица фильтра
    uint8_t r;              // Радиус

    double factor;          // Коэффициент сглаживания
    double bias;            // Смещение
} Filter;

#endif