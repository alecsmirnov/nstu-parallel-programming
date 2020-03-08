#include "mapreduce.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Указатель на фукнцию map/reduce
typedef double (*func_ptr)();

// Блоки данных, на которые поделит ф-ия map
typedef struct MPChunk {
    func_ptr func;              // Ф-ия для обработки

	double* A;                  // Преобразованный массив после ф-ии map 
    uint32_t size;              // Размер массива

    double result;              // Результат работы ф-й map и reduce
} MPChunk;

// Данные программы 
typedef struct MPData {
    uint8_t threads_count;      // Кол-во потоков
    pthread_t* threads;
    struct MPChunk* chunks;     // Блоки данных

    map_func_ptr map;           // Ф-ия map
    reduce_func_ptr reduce;     // Ф-ия reduce

    double* A;
    uint32_t size;

    double result;
} MPData;

static void mpDataChunksInit(MPData* mp_data) {
    // Разбиение данных на равные блоки
	int chunk_size = mp_data->size / mp_data->threads_count;
	int remainder = mp_data->size % mp_data->threads_count;

    // Дополнение блоков оставшимися элементами
    int err = 0;
	for (uint32_t i = 0, shift = 0, part_size = chunk_size; 
		 i != mp_data->threads_count; 
		 ++i, shift += part_size, part_size = chunk_size) {
		if (remainder) {
			--remainder;
			++part_size;
		}

        // Инициализация блоков данных
		mp_data->chunks[i].A = &mp_data->A[shift];
        mp_data->chunks[i].size = part_size;
    }
}

// Инициализация данных
static void mpDataInit(MPData* mp_data, double* A, uint32_t size, 
                       map_func_ptr map, reduce_func_ptr reduce, 
                       uint8_t threads_count) {
    mp_data->A = A;
    mp_data->size = size;
    mp_data->result = 0;

    mp_data->map = map;
    mp_data->reduce = reduce;

    mp_data->threads_count = threads_count < size ? threads_count : size;
    if (mp_data->size < mp_data->threads_count)
        mp_data->threads_count = 1;

    mp_data->threads = (pthread_t*)malloc(sizeof(pthread_t) * mp_data->threads_count);
    if (mp_data->threads == NULL)
        throwErr("Error: threads out of memmory!");
    
    // Создание блоков для обработки
    mp_data->chunks = (MPChunk*)malloc(sizeof(MPChunk) * mp_data->threads_count);
    if (mp_data->chunks == NULL)
        throwErr("Error: chunks out of memmory!");

    mpDataChunksInit(mp_data);
}

// Функция потока map
static void* threadFuncMap(void* arg) {
	MPChunk* chunk = (MPChunk*)arg;

    // Обработка каждого элемента
	for (uint32_t i = 0; i != chunk->size; ++i)
		chunk->A[i] = chunk->func(chunk->A[i]);

	pthread_exit(NULL);
}

// Потоковая обработка массива функцией map
static void releaseMap(MPData* mp_data) {

    // Разбиение данных на блоки
	int chunk_size = mp_data->size / mp_data->threads_count;
	int remainder = mp_data->size % mp_data->threads_count;

    int err = 0;
	for (uint8_t i = 0; i != mp_data->threads_count; ++i) {
        // Определение ф-ии
        mp_data->chunks[i].func = mp_data->map;

		err = pthread_create(&mp_data->threads[i], NULL, threadFuncMap, 
                             (void*)&mp_data->chunks[i]);
		if (err != 0)
            throwErr("Error: cannot create a map thread!");
	}

	for (uint8_t i = 0; i != mp_data->threads_count; ++i) {
		err = pthread_join(mp_data->threads[i], NULL);
		if (err != 0)
            throwErr("Error: cannot join a map thread!");
	}
}

// Функция потока reduce
static void* threadFuncReduce(void* arg) {
	MPChunk* chunk = (MPChunk*)arg;

    // Объединение данных
	for (uint32_t i = 0; i != chunk->size - 1; ++i)
        chunk->result = chunk->func(chunk->result, chunk->A[i + 1]);

	pthread_exit(NULL);
}

// Потоковая обработка массива функцией reduce
static void releaseReduce(MPData* mp_data) {
    int err = 0;
    for (uint8_t i = 0; i != mp_data->threads_count; ++i) {
        // Определение ф-ии
        mp_data->chunks[i].func = mp_data->reduce;
        // Инициализация результирующей переменной блока
        mp_data->chunks[i].result = mp_data->chunks[i].A[0];

    	err = pthread_create(&mp_data->threads[i], NULL, threadFuncReduce, 
                             (void*)&mp_data->chunks[i]);
		if (err != 0)
            throwErr("Error: cannot create a reduce thread!");
    }
    
    for (uint8_t i = 0; i != mp_data->threads_count; ++i) {
		err = pthread_join(mp_data->threads[i], NULL);
		if (err != 0)
            throwErr("Error: cannot join a reduce thread");
	}

    // Объединение результатов из блоков, после обработки reduce
    mp_data->result = mp_data->chunks[0].result;
    for (uint8_t i = 0; i != mp_data->threads_count - 1; ++i)
        mp_data->result = mp_data->reduce(mp_data->result, 
                                          mp_data->chunks[i + 1].result);
}

// Очистка блоков
static void mpDataClear(MPData* mp_data) {
    free(mp_data->threads);
    free(mp_data->chunks);
}

// Обработка массива по модели mapReduce
double mapReduceArray(double* A, uint32_t size, 
                      map_func_ptr map, reduce_func_ptr reduce, 
                      uint8_t threads_count) {
    MPData mp_data;
    mpDataInit(&mp_data, A, size, map, reduce, threads_count);

    if (map)
        releaseMap(&mp_data);

    if (reduce)
        releaseReduce(&mp_data);

    mpDataClear(&mp_data);

    return mp_data.result;
}