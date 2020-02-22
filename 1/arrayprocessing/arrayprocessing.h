#ifndef ARRAYPROCESSING_H
#define ARRAYPROCESSING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef double (*func_ptr)(double);

#define arrayCreate(array_size) \
	(double*)malloc(sizeof(double) * array_size)

#define arrayInit(A, array_size) do {			\
	for (uint32_t i = 0; i != array_size; ++i)	\
		A[i] = rand() % array_size;				\
} while (0)

#define arrayCopy(dest, src, array_size) do {	\
	for (uint32_t i = 0; i != array_size; ++i)	\
		dest[i] = src[i];						\
} while (0)

arryPrint(fp, A, array_size) do {				\
	for (uint32_t i = 0; i != array_size; ++i)	\
		fprintf(fp, "%g ", A[i]);				\
	fprintf(fp, "\n");							\
} while (0)

void arrayProcessing(double* A, uint32_t array_size, uint8_t threads_count, func_ptr func);

#endif
