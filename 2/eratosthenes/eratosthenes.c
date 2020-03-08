#include "eratosthenes.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define BILLION 1.0E+9

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Функция вычета разности между временными величинами
#define clocktimeDifference(start, stop)            \
	1.0 * (stop.tv_sec - start.tv_sec) +            \
    1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

// Определение размера блока для потока
#define chunkSize(i, chunk_size, n) \
    (i + chunk_size < n ? chunk_size : n - i)

// Статус потока
typedef enum WorkStatus {
    BUSY,                   // Занят
    READY,                  // Готов
    OFF                     // Завершён
} WorkStatus;

// Данные потока
typedef struct Work {
    bool* data;             // Общие данные

    size_t begin;           // Индексы блока для обработки
    size_t end;
    size_t sieve_val;       // Число для исключения

    WorkStatus status;      // Статус потока

    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Work;

// Данные программы
typedef struct Prime {
    uint8_t threads_count;  // Кол-во потоков
    size_t n;               // Кол-во чисел
    size_t chunk_size;      // Размер блока для потока

    bool* data;             // Общие данные (список чисел)

    pthread_t* threads;     // Потоки
    struct Work* works;     // Данные потоков
} Prime;

// Инициализация данных
static void primeDataInit(PrimeNumbers* prime_numbers, Prime* prime, 
                          uint8_t threads_count, size_t n, size_t chunk_size) {
    prime->threads_count = threads_count;
    prime->n = n - FIRST_VALUE;
    prime->chunk_size = chunk_size;

    prime->data = (bool*)malloc(sizeof(bool) * prime->n);
    if (prime->data == NULL)
        throwErr("Error: data out of memmory!");

    memset(prime->data, true, sizeof(bool) * prime->n);

    prime->threads = (pthread_t*)malloc(sizeof(pthread_t) * prime->threads_count);
    if (prime->threads == NULL)
        throwErr("Error: threads out of memmory!");

    prime->works = (Work*)malloc(sizeof(Work) * prime->threads_count);
    if (prime->works == NULL)
        throwErr("Error: works out of memmory!");
    
    // Инициализация потоков для готовности к работе
    int err = 0;
    for (uint8_t i = 0; i != prime->threads_count; ++i) {
        prime->works[i].data = prime->data;

        prime->works[i].begin = 0;
        prime->works[i].end = 0;
        prime->works[i].sieve_val = 0;

        prime->works[i].status = READY;

        err = pthread_mutex_init(&prime->works[i].mutex, NULL);
        if (err != 0)
            throwErr("Error: cannot initialize push mutex!");

        err = pthread_cond_init(&prime->works[i].cond, NULL); 
        if (err != 0)
		    throwErr("Error: cannot initialize conditin variable!");
    }

    prime_numbers->data = prime->data;
    prime_numbers->n = prime->n;
    prime_numbers->elapsed_time = 0;
}

// Фильтрация данных потока
static void sieveDataChunk(Work* work) {
    // Поиск первого кратного простого числа в блоке
    while (work->begin < work->end && (
           work->data[work->begin] == 0 || valFromIndex(work->begin) % work->sieve_val != 0))
        ++work->begin;

    // Фильтрация данные в блоке с шагом sieve_val
    for (size_t i = work->begin; i < work->end; i += work->sieve_val)
        if (valFromIndex(i) % work->sieve_val == 0 && valFromIndex(i) != work->sieve_val)
            work->data[i] = false;
}

// Функция потока
static void* threadWorker(void* arg) {
    Work* work = (Work*)arg;
    int err = 0;

    // Пока поток не завершён, ждём данные и фильтруем их
    do {
        err = pthread_mutex_lock(&work->mutex);
        if (err != 0)
            throwErr("Error: mutex lock!");

        while (work->status == READY) {
            err = pthread_cond_wait(&work->cond, &work->mutex);
            if (err != 0)
                throwErr("Error: cannot wait on cond variable!");
        }

        // Если поток занят -- выполняем работу и освобождаем его
        if (work->status == BUSY) {
            sieveDataChunk(work);

            work->status = READY;
        }

        err = pthread_mutex_unlock(&work->mutex);
        if (err != 0)
            throwErr("Error: mutex unlock!");
    } while (work->status != OFF);
    
    pthread_exit(NULL);
}

// Получение готового к работе потока
static Work* getReadyThread(Prime* prime) {
    Work* work = NULL;

    do {
        bool found = false;
        for (uint8_t j = 0; j != prime->threads_count && !found; ++j)
            if (prime->works[j].status == READY) {
                work = &prime->works[j];
                found = true;
            }
    } while (work == NULL);

    return work;
}

// Получение следующего числа для фильтрации
static size_t getNextSieveVal(Prime* prime, size_t cur_sieve_val) {
    size_t sieve_val = cur_sieve_val;

    bool found = false;
    for (size_t i = sieve_val - 1; i < prime->n && !found; ++i)
        if (prime->data[i] && sieve_val < valFromIndex(i)) {
            sieve_val = valFromIndex(i);
            found = true;
        }

    return sieve_val;
}

// Завершение работы потоков
static void closeThreads(Prime* prime) {
    size_t closed_count = 0;
    int err = 0;

    // Если тред не занят (готов к работе),
    // присваиваем завершающий статус и посылаем сигнал
    while (closed_count != prime->threads_count) 
        for (uint8_t i = 0; i != prime->threads_count; ++i)
            if (prime->works[i].status == READY) {
                err = pthread_mutex_lock(&prime->works[i].mutex);
                if (err != 0)
                    throwErr("Error: mutex lock!");

                ++closed_count;
                prime->works[i].status = OFF;

                pthread_cond_signal(&prime->works[i].cond);

                err = pthread_mutex_unlock(&prime->works[i].mutex);
                if (err != 0)
                    throwErr("Error: mutex unlock!");
            }
}

// Создание работы для потоков
static void produceWork(Prime* prime) {
    size_t sieve_val = FIRST_VALUE;
    int err = 0;

    // Пока число для фильтрации меньше чем sqrt(n)
    while (sieve_val * sieve_val < prime->n) {
        // Делим общие данные на блоки
        for (size_t i = 0; i < prime->n; i += prime->chunk_size) {
            // Получаем готовые к работе поток
            Work* work = getReadyThread(prime);

            err = pthread_mutex_lock(&work->mutex);
            if (err != 0)
                throwErr("Error: mutex lock!");
            
            // Передаём данные потоку и присваиваем стутс "занят"
            work->begin = i;
            work->end = i + chunkSize(i, prime->chunk_size, prime->n);
            work->sieve_val = sieve_val;

            work->status = BUSY;

            pthread_cond_signal(&work->cond);

            err = pthread_mutex_unlock(&work->mutex);
            if (err != 0)
                throwErr("Error: mutex unlock!");
        }

        // Если все данные, с заданным числом для фильтрации, обработаны
        // Находим новое число для фильтрации 
        sieve_val = getNextSieveVal(prime, sieve_val);
    }

    // Закрываем потоки
    closeThreads(prime);
}

// Очищаем все данные кроме списка чисел
static void primeDataFree(Prime* prime) {
    for (uint8_t i = 0; i != prime->threads_count; ++i) {
        pthread_mutex_destroy(&prime->works[i].mutex);
        pthread_cond_destroy(&prime->works[i].cond); 
    }

    free(prime->threads);
    free(prime->works);
}

// Поиск простых чисел по: кол-ву потоков, числу n, размеру блока для потока
PrimeNumbers sieveStart(uint8_t threads_count, size_t n, size_t chunk_size) {
    PrimeNumbers prime_numbers;
    Prime prime;

    // Инициализация данных
    primeDataInit(&prime_numbers, &prime, threads_count, n, chunk_size);

    struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

    // Создание потоков для обработки поступающей работы
    int err = 0;
    for (uint8_t i = 0; i != prime.threads_count; ++i) {
		err = pthread_create(&prime.threads[i], NULL, threadWorker, (void*)&prime.works[i]);
        if (err != 0)
		    throwErr("Error: cannot create a thread!");
	}
    
    // Создание работы для потоков
    produceWork(&prime);

    for (uint8_t i = 0; i != prime.threads_count; ++i) {
		err = pthread_join(prime.threads[i], NULL);
		 if (err != 0)
		    throwErr("Error: cannot join a thread!");
	}

    clock_gettime(CLOCK_MONOTONIC, &stop);
    prime_numbers.elapsed_time = clocktimeDifference(start, stop);

    primeDataFree(&prime);

    return prime_numbers;
} 