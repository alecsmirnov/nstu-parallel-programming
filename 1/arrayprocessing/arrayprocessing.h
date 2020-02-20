#ifndef ARRAYPROCESSING_H
#define ARRAYPROCESSING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef double (*func_ptr)(double);

static inline double* arrayCreate(uint32_t array_size) {
	double* A = (double*)malloc(sizeof(double) * array_size);

	return A;
}

static inline double* arrayInit(uint32_t array_size) {
	double* A = (double*)malloc(sizeof(double) * array_size);

	for (uint32_t i = 0; i != array_size; ++i)
		A[i] = rand() % array_size;

	return A;
}

static inline void arrayCopy(double* dest, double* src, uint32_t array_size) {
	for (uint32_t i = 0; i != array_size; ++i)
		dest[i] = src[i];
}

static inline void arryPrint(double* A, uint32_t array_size) {
	for (uint32_t i = 0; i != array_size; ++i)
		printf("%g ", A[i]);

	printf("\n");
}

void arrayProcessing(double* A, uint32_t array_size, uint8_t threads_count, func_ptr func);

#endif
