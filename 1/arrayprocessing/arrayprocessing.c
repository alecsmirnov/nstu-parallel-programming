#include "arrayprocessing.h"

#include <pthread.h>

#include "errorhandle.h"

typedef struct ThreadParam {
	double* A;
	uint32_t size;

	func_ptr func;
} ThreadParam;

static void* threadFunc(void* arg) {
	ThreadParam* thread_param = (ThreadParam*)arg;

	for (uint32_t i = 0; i != thread_param->size; ++i)
		thread_param->A[i] = thread_param->func(thread_param->A[i]);
}

void arrayProcessing(double* A, uint32_t array_size, uint8_t threads_count, func_ptr func) {
	// Определяем переменные: информация о потоке (идентификатор потока и номер) и код ошибки
	pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * threads_count);
	ThreadParam* threads_param = (ThreadParam*)malloc(sizeof(ThreadParam) * threads_count);

	int chunk_size = array_size / threads_count;
	int remainder = array_size % threads_count;

	int err;
	// Создаём потоки и разбиваем массив на чанки
	for (uint32_t i = 0, shift = 0, part_size = chunk_size; i != threads_count; ++i, shift += part_size, part_size = chunk_size) {
		if (remainder) {
			--remainder;
			++part_size;
		}

		threads_param[i] = (ThreadParam){A + shift, part_size, func};

		err = pthread_create(&threads[i], NULL, threadFunc, (void*)&threads_param[i]);
		errorHandle(err, PASS_CODE, "Cannot create a thread: ");
	}

	// Ожидаем завершения созданных потоков перед завершением работы программы
	for (uint8_t i = 0; i != threads_count; ++i) {
		err = pthread_join(threads[i], NULL);
		errorHandle(err, PASS_CODE, "Cannot join a thread");
	}

	free(threads);
	free(threads_param);
}