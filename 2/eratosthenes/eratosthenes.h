#ifndef ERATOSTHENES_H
#define ERATOSTHENES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Первое простое число в списке
#define FIRST_VALUE 2

// Список простых чисел
typedef struct PrimeNumbers {
    bool* data;             // Статус числа в списке
    size_t n;               // Кол-во чисел списка
} PrimeNumbers;

// Получить число из списка по индексу
static inline size_t valFromIndex(size_t i) {
    return i + FIRST_VALUE;
}

// Вывод простых чисел на экран
static inline void printPrimeNumbers(const PrimeNumbers* prime_numbers) {
    for (size_t i = 0; i != prime_numbers->n; ++i)
        if (prime_numbers->data[i])
            printf("%zu ", valFromIndex(i));
    printf("\n");
}

// Поиск простых чисел по: кол-ву потоков, числу n, размеру блока для потока
PrimeNumbers sieveStart(uint8_t threads_count, size_t n, size_t chunk_size);

#endif