#include <stdio.h>
#include <time.h>

#include "mapreduce.h"

#define DEMO_ARGS_COUNT 3
#define TEST_ARGS_COUNT 5

#define RESULT_FILENAME "result.txt"

#define BILLION 1.0E+9

#define clocktimeDifference(start, stop)            \
	1.0 * (stop.tv_sec - start.tv_sec) +            \
	1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

// Функция map
// Входные данные:
//     void* data;				 Массив данных
//     size_t size;				 размер массива
// Выходные данные:
//     KeyVal* key_val;   	     Список ключей-значений
void map(MapArg* arg) {
	// Преобразование массива к исходному типу
	double* val = (double*)arg->val;
	for (size_t i = 0; i != arg->size; ++i)
		++val[i];

	uint8_t* key = (uint8_t*)malloc(sizeof(uint8_t));
	*key = 1;

	// Добавление результата к списку ключей-значений
	emitIntermediate(&arg->key_val, (void*)key, (void*)val, arg->size);
}

// Функция reduce
// Входные данные:
//     KeyVal* key_val;   	     Список ключей-значений
// Выходные данные:
//     KeyVal* collection;       Результат ключей-значений
void reduce(ReduceArg* arg) {
	// Создание результат типа double*
	double* result = (double*)malloc(sizeof(double));
	*result = 1;

	// Прохождение по всему списку ключей-значений
	KeyValNode* iter = arg->key_val;
	
	while (iter) {
		uint8_t key = *(uint8_t*)iter->key;

		// Проверка ключа для демонстрации
		if (key == 1) {
			double* iter_val = (double*)iter->val;

			for (size_t i = 0; i != iter->size; ++i)
				*result *= iter_val[i];
		}

		iter = iter->next;
	}

	// Создание нового ключа для формирования коллекций
	uint8_t* key = (uint8_t*)malloc(sizeof(uint8_t));
	*key = 1;

	// Добавление результата в выходной список данных
	emitIntermediate(&arg->collection, (void*)key, (void*)result, 1);
}

// Функция обработки результатов потоков программы
// Входные данные:
//    KeyValNode** collections;         Массив списков ключей-значений
// Выходные данные:
//    void*                             Любые данные
void* merge(KeyValNode** collections, uint8_t count) {
	// Создание результат типа double*
	double* result = (double*)malloc(sizeof(double));
	*result = 1;

	// Прохождение по всем спискам ключей-значений
	for (uint8_t i = 0; i != count; ++i) {
		KeyValNode* iter = collections[i];

		while (iter) {
			uint8_t key = *(uint8_t*)iter->key;

			// Слияние всех результатов в один
			if (key == 1) {
				double *iter_val = (double*)iter->val;
				*result *= *iter_val;
			}

			iter = iter->next;
		}
	}

	return (void*)result;
}

static double* arrayCreate(uint32_t size) {
	double* A = (double*)malloc(sizeof(double) * size);
	return A;
}

static void arrayRandInit(double* A, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		A[i] = rand() % size;
}

static void arrayCopy(double* dest, double* src, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		dest[i] = src[i];
}

static void arryPrint(double* A, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		printf("%g ", A[i]);
	printf("\n");
}

static void testResultOutput(FILE* fp, uint8_t threads_count, 
                             size_t array_size_min, size_t array_size_max, 
							 size_t measure_count) {
	fprintf(fp, "size:\tthreads: 1\tthreads: 2\tthreads: 3\tthreads: 4\n");

	for (size_t size = array_size_min; size < array_size_max; size *= 10) {
		double* A_src = arrayCreate(size);
		double* A = arrayCreate(size);

		arrayRandInit(A_src, size);

		fprintf(fp, "%zu\t", size);

		for (uint8_t i = 0; i != threads_count; ++i) {
			arrayCopy(A, A_src, size);

			double elapsed_time = 0;
			for (size_t j = 0; j != measure_count; ++j) {
				struct timespec start, stop;
				clock_gettime(CLOCK_MONOTONIC, &start);

				void* result = mapReduceChunk((void*)A, sizeof(double), size,
						                      map, reduce, merge, i + 1);	

				clock_gettime(CLOCK_MONOTONIC, &stop);
				elapsed_time += clocktimeDifference(start, stop);

				free(result);
			}

			fprintf(fp, "%lf\t", elapsed_time / measure_count);
		}

		fprintf(fp, "\n");

		free(A_src);
		free(A);
	}
}

static void test(int argc, char* argv[]) {
	if (argc < TEST_ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <array size min> " 
						"<array size max> <measure count>\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	uint8_t threads_count = atoi(argv[1]);
	size_t array_size_min = atoi(argv[2]);
	size_t array_size_max = atoi(argv[3]);
	size_t measure_count = atoi(argv[4]);

	FILE* fp = fopen(RESULT_FILENAME, "w");

	printf("Program execution...\n");
	testResultOutput(fp, threads_count, array_size_min, 
	                 array_size_max, measure_count);
	printf("Done.\n");

	fclose(fp);
}

static void demonstration(int argc, char* argv[]) {
	if (argc < DEMO_ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <array size>\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

    uint8_t threads_count = atoi(argv[1]);
	size_t size = atoi(argv[2]);

	double* A = arrayCreate(size);
	arrayRandInit(A, size);

	printf("Source array:\n");
	arryPrint(A, size);

	void* result = mapReduceChunk((void*)A, sizeof(double), size,
						          map, reduce, merge, threads_count);	

	printf("Transformed array:\n");
	arryPrint(A, size);

	printf("\nMapReduce result:\n");
	if (result)
		printf("%g\n", *(double*)result);

	free(result);
	free(A);
}

int main(int argc, char* argv[]) {
	#ifdef TEST 
	test(argc, argv);
	#else 
	demonstration(argc, argv);
	#endif

    return 0;
}