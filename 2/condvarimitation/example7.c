#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h> 

#include "condvarimitation.h"

#define err_exit(code, str) do { 						\
	fprintf(stderr, "%s: %s\n", str, strerror(code));	\
	exit(EXIT_FAILURE);									\
} while (0);

int store;
enum store_state {EMPTY, FULL} state = EMPTY;

pthread_mutex_t mutex;

#ifdef MYCONDVAR
	CondType cond;
#else
	pthread_cond_t cond;
#endif

void* producer(void *arg) {
	int err;

	while (1) {
		// Захватываем мьютекс и ожидаем освобождения склада
		err = pthread_mutex_lock(&mutex);
		if (err != 0)
			err_exit(err, "Cannot lock mutex");

		while (state == FULL) {
			#ifdef MYCONDVAR
				pthreadCondWait(&cond, &mutex);
			#else
				err = pthread_cond_wait(&cond, &mutex);
				if (err != 0)
					err_exit(err, "Cannot wait on condition variable");
			#endif
		}

		// Получен сигнал, что на складе не осталось товаров.
		// Производим новый товар.
		store = rand();
		state = FULL;

		printf("producing number %d\n", store);

		// Посылаем сигнал, что на складе появился товар.
		#ifdef MYCONDVAR
			pthreadCondSignal(&cond);
		#else
			err = pthread_cond_signal(&cond);
			if (err != 0)
				err_exit(err, "Cannot send signal");
		#endif

		err = pthread_mutex_unlock(&mutex);
		if (err != 0)
			err_exit(err, "Cannot unlock mutex");
	} 
}

void* consumer(void* arg) {
	int err;

	while (1) {
		// Захватываем мьютекс и ожидаем появления товаров на складе
		err = pthread_mutex_lock(&mutex);
		if (err != 0)
			err_exit(err, "Cannot lock mutex");

		while (state == EMPTY) {
			#ifdef MYCONDVAR
				pthreadCondWait(&cond, &mutex);
			#else
				err = pthread_cond_wait(&cond, &mutex);
				if (err != 0)
					err_exit(err, "Cannot wait on condition variable");
			#endif
		}

		// Получен сигнал, что на складе имеется товар.
		// Потребляем его.
		printf("Consuming number %d ", store);
		sleep(1);
		printf("done\n");
		state = EMPTY;

		// Посылаем сигнал, что на складе не осталось товаров.
		#ifdef MYCONDVAR
			pthreadCondSignal(&cond);
		#else
			err = pthread_cond_signal(&cond);
			if (err != 0)
				err_exit(err, "Cannot send signal");
		#endif

		err = pthread_mutex_unlock(&mutex);
		if (err != 0)
			err_exit(err, "Cannot unlock mutex");
	}
}

int main(int argc, char* argv[]) {
	pthread_t thread1, thread2;
	int err;

	#ifdef MYCONDVAR
		pthreadCondInit(&cond);
	#else
		err = pthread_cond_init(&cond, NULL); 
		if (err != 0)
			err_exit(err, "Cannot initialize condition variable");
	#endif

	err = pthread_mutex_init(&mutex, NULL);
	if (err != 0)
		err_exit(err, "Cannot initialize mutex");

	// Создаём потоки
	err = pthread_create(&thread1, NULL, producer, NULL);
	if(err != 0)
		err_exit(err, "Cannot create thread 1");
	err = pthread_create(&thread2, NULL, consumer, NULL);
	if(err != 0)
		err_exit(err, "Cannot create thread 2");

	// Дожидаемся завершения потоков
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	// Освобождаем ресурсы, связанные с мьютексом
	// и условной переменной
	pthread_mutex_destroy(&mutex);
	#ifdef MYCONDVAR
		pthreadCondDestroy(&cond);
	#else
		pthread_cond_destroy(&cond);
	#endif

	return 0;
} 