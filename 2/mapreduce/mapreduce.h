#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include <stdlib.h>
#include <stdint.h>

// Результать работы функций map/reduce
typedef struct KeyValNode {
    void* key;					// Ключ
	void* val;					// Данные
    size_t size;				// Размер

    struct KeyValNode* next;	// Следующий элемент
} KeyValNode;

// Аргумент функций map
typedef struct MapArg {
	// Входные данные:
	void* val;					// Данные
	size_t size;				// Размер

	// Выходные данные:
    KeyValNode* key_val;		// Список ключей-значений
} MapArg;

// Аргумент функций reduce
typedef struct ReduceArg {
	// Входные данные:
    KeyValNode* key_val;		// Список ключей-значений из функции map

	// Выходные данные:
	KeyValNode* collection;		// Сжатый список ключей значений
} ReduceArg;

// Указатели на функцию map, reduce, merge
typedef void (*map_ptr)(MapArg*);
typedef void (*reduce_ptr)(ReduceArg*);
typedef void* (*merge_ptr)(KeyValNode**, uint8_t);

// Запись результата функций map/reduce
void emitIntermediate(KeyValNode** result, void* key, void* val, size_t size);

// Обработка массива по модели mapReduce
void* mapReduceChunk(void* val, size_t val_size, size_t size,
					 map_ptr map, reduce_ptr reduce, merge_ptr merge,
					 uint8_t threads_count);

#endif