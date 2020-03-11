#ifndef ARRAYPROCESSING_H
#define ARRAYPROCESSING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Указатель на функцию обработки элемента массива
typedef double (*func_ptr)(double);

// Создание массива
static inline double* arrayCreate(size_t size) {
	double* A = (double*)malloc(sizeof(double) * size);
	return A;
}

// Инициализация массива случайными числами
static inline void arrayRandInit(double* A, size_t size) {
	for (size_t i = 0; i != size; ++i)
		A[i] = rand() % size;
}

// Копирование массива
static inline void arrayCopy(double* dest, double* src, size_t size) {
	for (size_t i = 0; i != size; ++i)
		dest[i] = src[i];
}

// Вывод массива на экран
static inline void arryPrint(double* A, size_t size) {
	for (size_t i = 0; i != size; ++i)
		printf("%g ", A[i]);
	printf("\n");
}

// Обработка элементов массива
void arrayProcessing(double* A, size_t size, func_ptr func, uint8_t threads_count);

#endif