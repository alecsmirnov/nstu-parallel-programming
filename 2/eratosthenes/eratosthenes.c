#include "eratosthenes.h"

#include <stdio.h>
#include <stdbool.h>

#define FIRST_NATURAL 2

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

#define chunkSize(i, chunk_size, n) \
    (i + chunk_size < n ? chunk_size : n - i)

typedef enum WorkStatus {
    BUSY,
    READY,
    OFF
} WorkStatus;

struct Work {
    WorkStatus status;

    size_t sieve_val;
    size_t* data;
    size_t size;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void primeDataInit(Prime* prime, uint8_t threads_count, size_t n, size_t chunk_size) {
    prime->threads_count = threads_count;
    prime->n = n - 2;
    prime->chunk_size = chunk_size;

    prime->data = (size_t*)malloc(sizeof(size_t) * prime->n);
    if (prime->data == NULL)
        throwErr("Error: data out of memmory!");

    for (size_t i = 0; i != prime->n; ++i)
        prime->data[i] = i + 2;

    prime->threads = (pthread_t*)malloc(sizeof(pthread_t) * prime->threads_count);
    if (prime->threads == NULL)
        throwErr("Error: threads out of memmory!");

    prime->works = (Work*)malloc(sizeof(Work) * prime->threads_count);
    if (prime->works == NULL)
        throwErr("Error: works out of memmory!");
    
    int err = 0;
    for (uint8_t i = 0; i != prime->threads_count; ++i) {
        prime->works[i].status = BUSY;
        
        prime->works[i].sieve_val = 0;
        prime->works[i].data = NULL;
        prime->works[i].size = 0;

        err = pthread_mutex_init(&prime->works[i].mutex, NULL);
        if (err != 0)
            throwErr("Error: cannot initialize push mutex!");

        err = pthread_cond_init(&prime->works[i].cond, NULL); 
        if (err != 0)
		    throwErr("Error: cannot initialize conditin variable!");
    }
}

static void* threadWorker(void* arg) {
    Work* work = (Work*)arg;
    int err = 0;

    do {
        err = pthread_mutex_lock(&work->mutex);
        if (err != 0)
            throwErr("Error: mutex lock!");

        work->status = READY;
        while (work->status == READY) {
            err = pthread_cond_wait(&work->cond, &work->mutex);
            if (err != 0)
                throwErr("Error: cannot wait on cond variable!");
        }

        if (work->status == BUSY) {
            size_t j = 0;
            while (j < work->size && (work->data[j] == 0 || work->data[j] % work->sieve_val != 0))
                ++j;
        
            for (size_t i = j; i < work->size; i += work->sieve_val)
                if (work->data[i] % work->sieve_val == 0 && work->data[i] != work->sieve_val)
                    work->data[i] = 0;
        }

        err = pthread_mutex_unlock(&work->mutex);
        if (err != 0)
            throwErr("Error: mutex unlock!");
    } while (work->status != OFF);
    
    pthread_exit(NULL);
}

static void produceWork(Prime* prime) {
    size_t sieve_val = FIRST_NATURAL;
    int err = 0;

    while (sieve_val * sieve_val < prime->n) {
        for (size_t i = 0; i < prime->n; i += prime->chunk_size) {
            Work* work = NULL;

            do {
                bool found = false;
                for (uint8_t j = 0; j != prime->threads_count && !found; ++j)
                    if (prime->works[j].status == READY) {
                        work = &prime->works[j];
                        found = true;
                    }
            } while (work == NULL);

            err = pthread_mutex_lock(&work->mutex);
            if (err != 0)
                throwErr("Error: mutex lock!");

            work->sieve_val = sieve_val;
            work->data = &prime->data[i];
            work->size = chunkSize(i, prime->chunk_size, prime->n);
            
            work->status = BUSY;

            pthread_cond_signal(&work->cond);

            err = pthread_mutex_unlock(&work->mutex);
            if (err != 0)
                throwErr("Error: mutex unlock!");
        }

        if (sieve_val * sieve_val < prime->n) {
            bool found = false;
            for (size_t i = 0; i < prime->n && !found; ++i)
                if (sieve_val < prime->data[i]) {
                    sieve_val = prime->data[i];
                    found = true;
                }
        }
    }

    for (uint8_t i = 0; i != prime->threads_count; ++i) {
        err = pthread_mutex_lock(&prime->works[i].mutex);
        if (err != 0)
            throwErr("Error: mutex lock!");

        prime->works[i].status = OFF;

        pthread_cond_signal(&prime->works[i].cond);

        err = pthread_mutex_unlock(&prime->works[i].mutex);
        if (err != 0)
            throwErr("Error: mutex unlock!");
    }
}

void sieveStart(Prime* prime, uint8_t threads_count, size_t n, size_t chunk_size) {
    int err = 0;
    for (uint8_t i = 0; i != prime->threads_count; ++i) {
		err = pthread_create(&prime->threads[i], NULL, threadWorker, (void*)&prime->works[i]);
        if (err != 0)
		    throwErr("Error: cannot create a thread!");
	}
    
    produceWork(prime);

    for (uint8_t i = 0; i != prime->threads_count; ++i) {
		err = pthread_join(prime->threads[i], NULL);
		 if (err != 0)
		    throwErr("Error: cannot join a thread!");
	}
} 

void primeDataFree(Prime* prime) {
    prime->threads_count = 0;
    prime->n = 0;
    prime->chunk_size = 0;

    for (uint8_t i = 0; i != prime->threads_count; ++i) {
        pthread_mutex_destroy(&prime->works[i].mutex);
        pthread_cond_destroy(&prime->works[i].cond); 
    }

    free(prime->data);
    free(prime->threads);
    free(prime->works);

    prime->data = NULL;
    prime->threads = NULL;
    prime->works = NULL;
}