#include "mapreduce.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Указатель на фукнцию map/reduce
typedef void (*func_ptr)();

// Аргументы потока
typedef struct ThreadArg {
    func_ptr func;              // Указатель на функцию map/reduce
    void* arg;                  // Аргумент функции
} ThreadArg;

// Данные программы 
typedef struct Data {
    void* val;                  // Данные
    size_t val_size;            // Размер типа данных
    size_t size;                // Размер данных

    uint8_t threads_count;      // Кол-во потоков
    pthread_t* threads;
    ThreadArg* threads_arg;

    MapArg* map_data;           // Аргументы функции map
    ReduceArg* reduce_data;     // Аргументы функции reduce

    KeyValNode** collections;   // Массив ключей значений
} Data;                         // (Результаты всех потоков после выполнения функции reduce)

// Инициализация данных
static void dataInit(Data* data, void* val, size_t val_size, size_t size, uint8_t threads_count) {
    data->val = val;
    data->val_size = val_size;
    data->size = size;

    data->threads_count = threads_count < size ? threads_count : size;
}

// Инициализация потоков
static void threadsInit(Data* data) {
    data->threads = (pthread_t*)malloc(sizeof(pthread_t) * data->threads_count);
    if (data->threads == NULL)
        throwErr("Error: threads out of memmory!");

    data->threads_arg = (ThreadArg*)malloc(sizeof(ThreadArg) * data->threads_count);
    if (data->threads_arg == NULL)
        throwErr("Error: threads arg out of memmory!");
}

// Инициализация данных функции map
static void mapDataInit(Data* data) {
    data->map_data = (MapArg*)malloc(sizeof(MapArg) * data->threads_count);
    if (data->map_data == NULL)
        throwErr("Error: mad data out of memmory!");

    // Определение размера блоков данных
	size_t chunk_size = data->size / data->threads_count;
	uint8_t remainder = data->size % data->threads_count;

    size_t shift = 0;
    for (uint8_t i = 0; i != data->threads_count; ++i) {
        data->map_data[i].val = data->val + data->val_size * shift;
        data->map_data[i].size = chunk_size + (i < remainder ? 1 : 0);
        
        data->map_data[i].key_val = NULL;

        shift += data->map_data[i].size;
    }
}

// Инициализация данных функции reduce
static void reduceDataInit(Data* data) {
    data->reduce_data = (ReduceArg*)malloc(sizeof(ReduceArg) * data->threads_count);
    if (data->reduce_data == NULL)
        throwErr("Error: reduce out of memmory!");

    // Передача списка ключей-значений из функции map в функцию reduce 
    for (size_t i = 0; i != data->threads_count; ++i) {
        data->reduce_data[i].key_val = data->map_data[i].key_val;

        data->reduce_data[i].collection = NULL;
    }
}

static void* threadFunc(void* arg) {
	ThreadArg* chunk = (ThreadArg*)arg;
    chunk->func(chunk->arg);

	pthread_exit(NULL);
}

// Выполнение функции map/reduce
static void releaseFunc(Data* data, func_ptr func, void* arg, size_t arg_size) {
    int err = 0;
	for (uint8_t i = 0; i != data->threads_count; ++i) {
        // Инициализация потоков переданными данными
        data->threads_arg[i].func = func;                       // Функция
        data->threads_arg[i].arg = arg + arg_size * i;          // Аргумент функции

		err = pthread_create(&data->threads[i], NULL, threadFunc, (void*)&data->threads_arg[i]);
		if (err != 0)
            throwErr("Error: cannot create a map thread!");
	}

	for (uint8_t i = 0; i != data->threads_count; ++i) {
		err = pthread_join(data->threads[i], NULL);
		if (err != 0)
            throwErr("Error: cannot join a map thread!");
	}
}

// Формирование коллекциии результатов после выполнения функции reduce
static void makeCollections(Data* data) {
    // Создание массива результатотв потоков
    data->collections = (KeyValNode**)malloc(sizeof(KeyValNode*) * data->threads_count);
    if (data->collections == NULL)
        throwErr("Error: collections out of memmory!");

    for (uint8_t i = 0; i != data->threads_count; ++i)
        data->collections[i] = data->reduce_data[i].collection;
}

// Очистка списков ключей-значений
static void keyValClear(KeyValNode* key_val_data, bool clear_val) {
    KeyValNode* iter = key_val_data;

    while (iter) {
        KeyValNode* prev = iter;
        iter = iter->next;
        
        if (prev->key) {
            free(prev->key);
            prev->key = NULL;
        }

        if (clear_val)
            if (prev->val) {
                free(prev->val);
                prev->key = NULL;
            }

        free(prev); 
    }
}

// Очистка всех данных
static void dataClear(Data* data) {
    free(data->threads);
    free(data->threads_arg);
    
    for (uint8_t i = 0; i != data->threads_count; ++i) {
        keyValClear(data->map_data[i].key_val, false);
        keyValClear(data->reduce_data[i].collection, true);
    }
    
    free(data->map_data);
    free(data->reduce_data);

    free(data->collections);
}

// Запись результата функций map/reduce
void emitIntermediate(KeyValNode** result, void* key, void* val, size_t size) {
    KeyValNode* new_node = (KeyValNode*)malloc(sizeof(KeyValNode));
    if (new_node == NULL)
        throwErr("Error: new node out of memmory!");

    new_node->key = key;
    new_node->val = val;
    new_node->size = size;
    
    new_node->next = *result;
    *result = new_node;
}

// Обработка массива по модели mapReduce
void* mapReduceChunk(void* val, size_t val_size, size_t size,
                     map_ptr map, reduce_ptr reduce, merge_ptr merge,
                     uint8_t threads_count) {
    Data data;

    dataInit(&data, val, val_size, size, threads_count);
    threadsInit(&data);

    mapDataInit(&data);
    releaseFunc(&data, map, data.map_data, sizeof(MapArg));

    reduceDataInit(&data);
    releaseFunc(&data, reduce, data.reduce_data, sizeof(ReduceArg));

    makeCollections(&data);

    void* result = merge(data.collections, data.threads_count);

    dataClear(&data);

    return result;
}