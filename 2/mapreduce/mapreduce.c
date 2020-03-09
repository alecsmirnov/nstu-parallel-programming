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
typedef void (*mrfunc_ptr)();

// Аргументы потока
typedef struct ThreadArg {
    mrfunc_ptr func;            // Указатель на ф-ию
    void* arg;                  // Аргумент ф-ии
} ThreadArg;

// Данные программы 
typedef struct MRData {
    uint8_t threads_count;      // Кол-во потоков
    pthread_t* threads;

    MRArg* chunks;              // Блоки данных
    MRArg* chunks_result;       // Результаты ф-ии reduce

    mfunc_ptr map;              // Ф-ия map
    rfunc_ptr reduce;           // Ф-ия reduce

    void* val;                  // Данные
    size_t size;                // Размер данных
    size_t data_size;           // Размер типа данных

    MRResult* result;
} MRData;

// Инициализация блоков данных
static void chunksInit(MRData* data) {
	int chunk_size = data->size / data->threads_count;
	int remainder = data->size % data->threads_count;

    // Дополнение блоков оставшимися элементами
    int err = 0;
	for (size_t i = 0, shift = 0, part_size = chunk_size; 
		 i != data->threads_count; 
		 ++i, shift += part_size, part_size = chunk_size) {
		if (remainder) {
			--remainder;
			++part_size;
		}

        // Инициализация блоков данных
		data->chunks[i].val = data->val + data->data_size * shift;
        data->chunks[i].size = part_size;
        data->chunks[i].key_val = NULL;
    }
}

// Инициализация данных
static void dataInit(MRData* data, double* A, size_t size, size_t data_size,
                       mfunc_ptr map, rfunc_ptr reduce, 
                       uint8_t threads_count) {
    data->val = A;
    data->size = size;
    data->data_size = data_size;

    data->result = NULL;

    data->map = map;
    data->reduce = reduce;

    data->threads_count = threads_count < size ? threads_count : size;

    data->threads = (pthread_t*)malloc(sizeof(pthread_t) * data->threads_count);
    if (data->threads == NULL)
        throwErr("Error: threads out of memmory!");
    
    data->chunks = (MRArg*)malloc(sizeof(MRArg) * data->threads_count);
    if (data->chunks == NULL)
        throwErr("Error: chunks out of memmory!");
    
    data->chunks_result = (MRArg*)malloc(sizeof(MRArg) * data->threads_count);
    if (data->chunks_result == NULL)
        throwErr("Error: chunks result out of memmory!");

    chunksInit(data);
}

static void* threadFunc(void* arg) {
	ThreadArg* chunk = (ThreadArg*)arg;
    chunk->func(chunk->arg);

	pthread_exit(NULL);
}

// Потоковая обработка массива функцией map
static void releaseMap(MRData* data) {
    ThreadArg* threads_arg = (ThreadArg*)malloc(sizeof(ThreadArg) * data->threads_count);
    if (threads_arg == NULL)
        throwErr("Error: map threads arg out of memmory!");

    int err = 0;
	for (uint8_t i = 0; i != data->threads_count; ++i) {
        threads_arg[i].func = data->map;
        threads_arg[i].arg = &data->chunks[i];

		err = pthread_create(&data->threads[i], NULL, threadFunc, (void*)&threads_arg[i]);
		if (err != 0)
            throwErr("Error: cannot create a map thread!");
	}

	for (uint8_t i = 0; i != data->threads_count; ++i) {
		err = pthread_join(data->threads[i], NULL);
		if (err != 0)
            throwErr("Error: cannot join a map thread!");
	}

    free(threads_arg);
}

// Формирование результата
static void formResult(MRResult** result, void* val, size_t size) {
    MRResultNode* new_node = (MRResultNode*)malloc(sizeof(MRResultNode));
    if (new_node == NULL)
        throwErr("Error: new node out of memmory!");

    new_node->val = val;
    new_node->size = size;

    new_node->next = *result;
    *result = new_node;
}

// Потоковая обработка массива функцией reduce
static void releaseReduce(MRData* data) {
    ThreadArg* threads_arg = (ThreadArg*)malloc(sizeof(ThreadArg) * data->threads_count);
    if (threads_arg == NULL)
        throwErr("Error: reduce threads arg out of memmory!");

    int err = 0;
 	for (uint8_t i = 0; i != data->threads_count; ++i) {
        data->chunks_result[i].val = NULL;
        data->chunks_result[i].size = 0;
        data->chunks_result[i].key_val = data->chunks[i].key_val;

        threads_arg[i].func = data->reduce;
        threads_arg[i].arg = &data->chunks_result[i];

		err = pthread_create(&data->threads[i], NULL, threadFunc, (void*)&threads_arg[i]);
		if (err != 0)
            throwErr("Error: cannot create a map thread!");
	}  
    
    for (uint8_t i = 0; i != data->threads_count; ++i) {
		err = pthread_join(data->threads[i], NULL);
		if (err != 0)
            throwErr("Error: cannot join a reduce thread");

        // Объединение всех результатов в один список
        formResult(&data->result, data->chunks_result[i].val, data->chunks_result[i].size);
	}

    free(threads_arg);
}

// Очистка блоков
static void dataClear(MRData* data) {
    free(data->threads);

    // Очистка ключей
    for (uint8_t i = 0; i != data->threads_count; ++i) {
        MRKeyValNode* iter = data->chunks[i].key_val;

        while (iter) {
            MRKeyValNode* prev = iter;
            iter = iter->next;

            if (prev->key)
                free(prev->key);
            free(prev); 
        }
    }

    free(data->chunks);
    free(data->chunks_result);
}

void mrEmitMap(MRArg** arg, void* key, void* val, size_t size) {
    MRKeyValNode* new_node = (MRKeyValNode*)malloc(sizeof(MRKeyValNode));
    if (new_node == NULL)
        throwErr("Error: new node out of memmory!");

    new_node->key = key;
    new_node->val = val;
    new_node->size = size;
    
    new_node->next = (*arg)->key_val;
    (*arg)->key_val = new_node;
}

void mrEmitReduce(MRArg** arg, void* val, size_t size) {
    (*arg)->val = val;
    (*arg)->size = size;
}

// Обработка массива по модели mapReduce
MRResult* mrArray(void* A, size_t size, size_t data_size,
                 mfunc_ptr map, rfunc_ptr reduce, 
                 uint8_t threads_count) {
    MRData data;
    dataInit(&data, A, size, data_size, map, reduce, threads_count);

    if (map)
        releaseMap(&data);

    if (reduce)
        releaseReduce(&data);
    else 
        formResult(&data.result, data.val, data.size);

    dataClear(&data);

    return data.result;
}