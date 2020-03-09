#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include <stdlib.h>
#include <stdint.h>

// Результать работы функции map
typedef struct MRKeyValNode {
    void* key;					// Ключ
	
	void* val;					// Данные
    size_t size;				// Размер данных

    struct MRKeyValNode* next;	// Следующий элемент
} MRKeyValNode, MRKeyVal;

// Аргумент функций map/reduce
typedef struct MRArg {
	// Входные значения для ф-ии map
	// (Выходные значения для ф-ии reduce)
	void* val;
	size_t size;

	// Выходное значение для ф-ии map
	// (Входное значение для ф-ии reduce)
    MRKeyVal* key_val;
} MRArg;

// Результат работы программы
typedef struct MRResultNode {
	void* val;					// Данные
    size_t size;				// Размер данных

	struct MRResultNode* next;
} MRResultNode, MRResult;

// Указатели на функцию map, reduce
typedef void (*mfunc_ptr)(MRArg*);
typedef void (*rfunc_ptr)(MRArg*);

// Запись результата функций
void mrEmitMap(MRArg** arg, void* key, void* val, size_t size);
void mrEmitReduce(MRArg** arg, void* val, size_t size);

// Обработка массива по модели mapReduce
MRResult* mrArray(void* A, size_t size, size_t data_size,
				 mfunc_ptr map, rfunc_ptr reduce,
                 uint8_t threads_count);

#endif