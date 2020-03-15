#include "jacobi.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

double getWTime() {
    return omp_get_wtime();
}

double* jacobi(double** a, double* b, size_t n, uint8_t threads_count) {
    omp_set_num_threads(threads_count);

    double* x = (double*)malloc(sizeof(double) * n);
    if (x == NULL)
        throwErr("Error: x out of memmory!");

	double* temp_x = (double*)malloc(sizeof(double) * n);
    if (temp_x == NULL) 
        throwErr("Error: temp_x out of memmory!");
    
    memset(x, 0, sizeof(double) * n);

	double norm = 0;
    size_t iters_count = 0;

	do {
        #pragma omp parallel for num_threads(threads_count)
		for (size_t i = 0; i < n; ++i) {
			temp_x[i] = b[i];

			for (size_t j = 0; j < n; ++j)
				if (i != j)
					temp_x[i] -= a[i][j] * x[j];

			temp_x[i] /= a[i][i];
		}

        norm = 0;
        for (size_t i = 0; i < n; ++i) {
            if (norm < fabs(x[i] - temp_x[i]))
                norm = fabs(x[i] - temp_x[i]);

			x[i] = temp_x[i];
		}
		
        ++iters_count;
	} while (EPS < norm && iters_count < ITERS_COUNT_MAX);

	free(temp_x);

    return x;
}