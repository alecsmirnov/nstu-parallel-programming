#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#define ARGS_COUNT 4 

#define FIRST_NATURAL 2

#define PTHREAD_PASS 0

#define pthreadErrorHandle(test_code, pass_code, err_msg) do {		\
	if (test_code != pass_code) {									\
		fprintf(stderr, "%s: %s\n", err_msg, strerror(test_code));	\
		exit(EXIT_FAILURE);											\
	}																\
} while (0)

#define chunkSize(i, chunk_size, n) (i + chunk_size < n ? chunk_size : n - i)

typedef enum ThreadStat {
    BUSY,
    READY,
    OUT
} ThreadStat;

typedef struct ThreadArg {
    int num;
    ThreadStat status;

    size_t* data;
    size_t size;

    size_t* sieve_val;

    pthread_mutex_t* pool_mutex;
    pthread_mutex_t* data_mutex;
    pthread_cond_t* data_cond;
} ThreadArg;

typedef struct Prime {
    uint8_t threads_count;
    size_t n;
    size_t chunk_size;

    size_t sieve_val;
    size_t* data;

    pthread_t* threads;
    ThreadArg* threads_arg;

    pthread_mutex_t pool_mutex;
    pthread_mutex_t data_mutex;
    pthread_cond_t* threads_data_cond;
} Prime;

void primeDataInit(Prime* prime, uint8_t threads_count, size_t n, size_t chunk_size) {
    prime->threads_count = threads_count;
    prime->n = n - 2;
    prime->chunk_size = chunk_size;

    prime->sieve_val = FIRST_NATURAL;

    prime->data = (size_t*)malloc(sizeof(size_t) * prime->n);
    for (size_t i = 0; i != prime->n; ++i)
        prime->data[i] = i + 2;

    int err = pthread_mutex_init(&prime->pool_mutex, NULL);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot initialize pool mutex");
    err = pthread_mutex_init(&prime->data_mutex, NULL);
    pthreadErrorHandle(err, PTHREAD_PASS, "Cannot initialize data mutex");

    prime->threads = (pthread_t*)malloc(sizeof(pthread_t) * prime->threads_count);
    prime->threads_arg = (ThreadArg*)malloc(sizeof(ThreadArg) * prime->threads_count);
    prime->threads_data_cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * prime->threads_count);

    for (uint8_t i = 0; i != prime->threads_count; ++i) {
        err = pthread_cond_init(&prime->threads_data_cond[i], NULL); 
        pthreadErrorHandle(err, PTHREAD_PASS, "Cannot initialize conditin variable");

        prime->threads_arg[i].status = READY;

        prime->threads_arg[i].data = NULL;
        prime->threads_arg[i].size = 0;

        prime->threads_arg[i].sieve_val = &prime->sieve_val;
        
        prime->threads_arg[i].pool_mutex = &prime->pool_mutex;
        prime->threads_arg[i].data_mutex = &prime->data_mutex;
        prime->threads_arg[i].data_cond = &prime->threads_data_cond[i];
        prime->threads_arg[i].num = i;
    }
}

void* threadWorker(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;

    printf("%d\n", thread_arg->num);

    while (thread_arg->status != OUT) {
        pthread_mutex_lock(thread_arg->pool_mutex);

        while (thread_arg->status == READY)
            pthread_cond_wait(thread_arg->data_cond, thread_arg->pool_mutex);

        pthread_mutex_unlock(thread_arg->pool_mutex);

        if (thread_arg->status == BUSY) {
            bool found = false;
            size_t j = 0;
            while (j < thread_arg->size && (thread_arg->data[j] == 0 || thread_arg->data[j] % *thread_arg->sieve_val != 0))
                ++j;

            if (thread_arg->num == 1) {
                printf("j: %zu | sv: %zu | ts: %zu\n", thread_arg->data[j], *thread_arg->sieve_val, thread_arg->size);

                for (size_t i = 0; i != thread_arg->size; ++i)
                    printf("%zu ", thread_arg->data[i]);
                printf("\n");
            }

            for (size_t i = j; i < thread_arg->size; i += *thread_arg->sieve_val) {
                if (thread_arg->num == 1) {
                    printf("VAL: %zu | SIEVE: %zu | DIV: %zu | ",thread_arg->data[i], *thread_arg->sieve_val, *thread_arg->sieve_val % thread_arg->data[i]);
                    printf("EQV: %d\n", thread_arg->data[i] != *thread_arg->sieve_val);
                }
                if (thread_arg->data[i] % *thread_arg->sieve_val == 0 && thread_arg->data[i] != *thread_arg->sieve_val)
                    thread_arg->data[i] = 0;
            }

            pthread_mutex_lock(thread_arg->pool_mutex);
            thread_arg->status = READY;
            pthread_mutex_unlock(thread_arg->pool_mutex);
        }
    }

    pthread_exit(NULL);
}

ThreadArg* findFreeThread(Prime* prime) {
    ThreadArg* thread_arg = NULL;

    bool found = false;
    while (!found)
        for (uint8_t i = 0; i != prime->threads_count && !found; ++i) {
            pthread_mutex_lock(&prime->pool_mutex);

            if (prime->threads_arg[i].status == READY) {
                thread_arg = &prime->threads_arg[i];
                found = true;
            }

            pthread_mutex_unlock(&prime->pool_mutex);
        }

    return thread_arg;
}

void threadsWait(Prime* prime) {
    uint8_t ready_threads = 0;

    while (ready_threads < prime->threads_count)
        for (uint8_t i = 0; i != prime->threads_count; ++i) {
            pthread_mutex_lock(&prime->pool_mutex);

            if (prime->threads_arg[i].status == READY)
                ++ready_threads;

            pthread_mutex_unlock(&prime->pool_mutex);
        }
}

void sieveValNext(Prime* prime) {
    pthread_mutex_lock(&prime->data_mutex);

    bool found = false;
    for (size_t i = prime->sieve_val + 1; i < prime->n && !found; ++i)
        if (prime->data[i]) {
            prime->sieve_val = prime->data[i];
            found = true;
        }
    
    pthread_mutex_unlock(&prime->data_mutex);
}

void closeThreads(Prime* prime) {
    uint8_t closed_count = 0;
    
    while (closed_count < prime->threads_count)
        for (uint8_t i = 0; i != prime->threads_count; ++i) {
            pthread_mutex_lock(&prime->pool_mutex);

            if (prime->threads_arg[i].status == READY) {    
                prime->threads_arg[i].status = OUT;

                pthread_cond_signal(&prime->threads_data_cond[i]);

                ++closed_count;
            }

            pthread_mutex_unlock(&prime->pool_mutex);
        }
}

void produceWork(Prime* prime) {
    while (prime->sieve_val * prime->sieve_val < prime->n) {
        for (size_t i = 0; i < prime->n; i += prime->chunk_size) {
            ThreadArg* thread_arg = findFreeThread(prime);

            pthread_mutex_lock(thread_arg->data_mutex);

            thread_arg->status = BUSY;
            thread_arg->data = &prime->data[i];
            thread_arg->size = chunkSize(i, prime->chunk_size, prime->n);
            pthread_cond_signal(thread_arg->data_cond);

            pthread_mutex_unlock(thread_arg->data_mutex);
        }

        threadsWait(prime);
        sieveValNext(prime);
    }

    closeThreads(prime);
}

void sieveStart(uint8_t threads_count, size_t n, size_t chunk_size) {
    Prime prime;
    primeDataInit(&prime, threads_count, n, chunk_size);

    int err = PTHREAD_PASS;
    for (uint8_t i = 0; i != prime.threads_count; ++i) {
		err = pthread_create(&prime.threads[i], NULL, threadWorker, (void*)&prime.threads_arg[i]);
		pthreadErrorHandle(err, PTHREAD_PASS, "Cannot create a thread");
	}

    produceWork(&prime);
    
    for (uint8_t i = 0; i != prime.threads_count; ++i) {
		err = pthread_join(prime.threads[i], NULL);
		pthreadErrorHandle(err, PTHREAD_PASS, "Cannot join a thread");
	}

    for (size_t i = 0; i != prime.n; ++i)
        if (prime.data[i])
            printf("%zu ", prime.data[i]);
    printf("\n");
} 

int main(int argc, char* argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <n> <chunk size>\n");
		exit(EXIT_FAILURE);
	}

    uint8_t threads_count = atoi(argv[1]);
	size_t n = atol(argv[2]);
	size_t chunk_size = atol(argv[3]);

    sieveStart(threads_count, n, chunk_size);

    return 0;
}