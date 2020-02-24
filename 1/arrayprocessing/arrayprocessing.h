#ifndef ARRAYPROCESSING_H
#define ARRAYPROCESSING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Указатель на функцию обработки элемента массива
typedef double (*func_ptr)(double);

// Создание массива
static inline double* arrayCreate(uint32_t array_size) {
	double* A = (double*)malloc(sizeof(double) * array_size);
	return A;
}

// Инициализация массива случайными числами
static inline void arrayInit(double* A, uint32_t array_size) {
	for (uint32_t i = 0; i != array_size; ++i)
		A[i] = rand() % array_size;
}

// Копирование массива
static inline void arrayCopy(double* dest, double* src, uint32_t array_size) {
	for (uint32_t i = 0; i != array_size; ++i)
		dest[i] = src[i];
}

// Вывод массива на экран
static inline void arryPrint(double* A, uint32_t array_size) {
	for (uint32_t i = 0; i != array_size; ++i)
		printf("%g ", A[i]);
	printf("\n");
}

// Обработка элементов массива
void arrayProcessing(double* A, uint32_t array_size, uint8_t threads_count, func_ptr func);

#endif
