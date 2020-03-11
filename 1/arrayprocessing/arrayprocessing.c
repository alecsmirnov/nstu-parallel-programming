#include "arrayprocessing.h"

#include <string.h>
#include <pthread.h>

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Параметры потока (блок данных для обработки)
typedef struct Chunk {
	double* A;			// Указатель на массив
	size_t size;		// Размер массива

	func_ptr func;		// Функция обработки элемента
} Chunk;

static void* threadFunc(void* arg) {
	Chunk* chunk = (Chunk*)arg;

	for (size_t i = 0; i != chunk->size; ++i)
		chunk->A[i] = chunk->func(chunk->A[i]);

	pthread_exit(NULL);
}

// Обработка элементов массива
void arrayProcessing(double* A, size_t size, func_ptr func, uint8_t threads_count) {
	// Создаём необходимое количество потоков
	pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * threads_count);
	if (threads == NULL)
        throwErr("Error: threads out of memmory!");

	Chunk* chunks = (Chunk*)malloc(sizeof(Chunk) * threads_count);
	if (chunks == NULL)
        throwErr("Error: chunks out of memmory!");

	// Определяем размер блоков и остаток по размеру массива
	size_t chunk_size = size / threads_count;
	uint8_t remainder = size % threads_count;

	int err = 0;
	size_t shift = 0;
	// Создаём потоки и разбиваем массив на блоки
	for (uint8_t i = 0; i != threads_count; ++i) {
		// Записываем в параметр потока ту часть массива,
		// которую он будет обрабатывать, и указатель на функцию обработки
		chunks[i] = (Chunk){A + shift, chunk_size + (i < remainder ? 1 : 0), func};

		shift += chunks[i].size;

		err = pthread_create(&threads[i], NULL, threadFunc, (void*)&chunks[i]);
		if (err != 0)
        	throwErr("Error: cannot create a thread: ");
	}

	// Ожидаем завершения созданных потоков перед завершением работы программы
	for (uint8_t i = 0; i != threads_count; ++i) {
		err = pthread_join(threads[i], NULL);
		if (err != 0)
        	throwErr("Error: cannot join a thread");
	}

	free(threads);
	free(chunks);
}