#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Указатели на функцию map, reduce
typedef double (*map_func_ptr)(double value);
typedef double (*reduce_func_ptr)(double result, double value);

static inline double* arrayCreate(uint32_t size) {
	double* A = (double*)malloc(sizeof(double) * size);
	return A;
}

static inline void arrayRandInit(double* A, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		A[i] = rand() % size;
}

static inline void arrayCopy(double* dest, double* src, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		dest[i] = src[i];
}

static inline void arryPrint(double* A, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		printf("%g ", A[i]);
	printf("\n");
}

// Обработка массива по модели mapReduce
double mapReduceArray(double* A, uint32_t size, 
                      map_func_ptr map, reduce_func_ptr reduce, 
                      uint8_t threads_count);

#endif