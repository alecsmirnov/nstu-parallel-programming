#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#ifdef MYMPI
#include "mympi.h"
#else 
#include <mpi.h>
#endif

#define ARGS_COUNT 3

// Номер главного процесса
#define ROOT_RANK 0

#define BILLION 1.0E+9

// Функция вычета разности между временными величинами
#define clocktimeDifference(start, stop)            \
    1.0 * (stop.tv_sec - start.tv_sec) +            \
    1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

// Функция отлова ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Доступ к элементам матрицы
#define matAccess(mat, m, i, j) \
    ((mat)[(i) * (m) + (j)])

// Создание массива длины size
static double* arrayCreate(size_t size) {
    double* A = (double*)malloc(sizeof(double) * size);
    if (A == NULL)
        throwErr("Error: array out of memmory!");

    return A;
}

// Инициализация массива случайными числами
static void arrayRandInit(double* A, size_t size) {
    for (size_t i = 0; i < size; ++i)
        A[i] = rand() % size;
}

// Функция умножения матрицы на вектор
static void matVecMult(double* res, double* mat, double* vec, size_t n, size_t m) {
    if (res == NULL)
        throwErr("Error: res null ptr!");

    for (size_t i = 0; i < n; ++i) {
        res[i] = 0;

        for (size_t j = 0; j < m; ++j)
            res[i] += matAccess(mat, m, i, j) * vec[j];
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    int rank = 0;
    int size = 1;

    // Инициализация MPI
    #ifdef MYMPI
    myMPIInit(&argc, &argv);

    myMPICommRank(&rank);
    myMPICommSize(&size);
    #else 
    MPI_Status status;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    #endif

    // Проверка входных аргументон
    if (argc < ARGS_COUNT) {
        if (rank == ROOT_RANK) {
            fprintf(stderr, "Wrong number of arguments!\n");
            fprintf(stderr, "Enter: <n> <m>\n");
        }

        #ifdef MYMPI
        myMPIFinalize();
        #else
        MPI_Finalize();
        #endif
        exit(EXIT_FAILURE);
    }

    // Размер задачи
    size_t root_n = atoi(argv[1]);
    size_t root_m = atoi(argv[2]);

    // Исходная матрица
    double* root_mat = NULL;

    // Размер обрабатываемого блока процессом
    size_t n = 0;
    size_t m = root_m;

    // Данные процессов:
    // Часть исходной матрицы (Исходня матрица для ROOT_RANK)
    double* mat = NULL;
    // Исходный вектор
    double* vec = NULL;
    // Часть результата (Конечный результат для ROOT_RANK)
    double* res = NULL;

    // Генерация данных (матрица m * n, вектор m)
    if (rank == ROOT_RANK) {
        root_mat = arrayCreate(root_n * root_m);
        arrayRandInit(root_mat, root_n * root_m);

        vec = arrayCreate(root_m);
        arrayRandInit(vec, root_m);
    }
    
    // Синхронизация процессов перед началом замера времени
    #ifdef MYMPI
    myMPIBarrier();
    #else 
    MPI_Barrier(MPI_COMM_WORLD);
    #endif

    struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

    if (rank == ROOT_RANK) {
        // Если процесс главный --
        // Отправить вектор и его размер другим процессам 
        for (uint8_t i = 1; i < size; ++i) {
             #ifdef MYMPI
            myMPISend(&root_m, 1, sizeof(size_t), i, 0);
            myMPISend(vec, root_m, sizeof(double), i, 0);
            #else
            MPI_Send(&root_m, 1, MPI_UNSIGNED_LONG_LONG, i, 0, MPI_COMM_WORLD);
            MPI_Send(vec, root_m, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            #endif
        }
        
        // Определение размеров блоков данных (строк для обработки)
        size_t chunk_size = root_n / size;
        uint8_t remainder = root_n % size;

        size_t shift = 0;
        for (uint8_t i = 0; i < size; ++i) {
            size_t row_size = chunk_size + (i < remainder ? 1 : 0);

            // Распределение строк матрицы между процессами
            if (i != ROOT_RANK) {
                #ifdef MYMPI
                myMPISend(&row_size, 1, sizeof(size_t), i, 0);
                myMPISend(mat + shift, row_size * root_m, sizeof(double), i, 0);
                #else
                MPI_Send(&row_size, 1, MPI_UNSIGNED_LONG_LONG, i, 0, MPI_COMM_WORLD);
                MPI_Send(mat + shift, row_size * root_m, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
                #endif
            }
            else {
                // Если процесс главный -- обрабатывает часть исходной матрицы
                n = row_size;
                mat = root_mat;
            }
            
            shift += row_size * root_m;
        }
    }
    else {
        // Если процесс не главный --
        // Принимает вектор и его размер
        // Принимает матрицу и её размер
        #ifdef MYMPI
        myMPIRecv(&m, 1, sizeof(size_t), ROOT_RANK, 0);
        vec = arrayCreate(m);
        myMPIRecv(vec, m, sizeof(double), ROOT_RANK, 0);

        myMPIRecv(&n, 1, sizeof(size_t), ROOT_RANK, 0);
        mat = arrayCreate(n * m);
        myMPIRecv(mat, n * m, sizeof(double), ROOT_RANK, 0);
        #else
        MPI_Recv(&m, 1, MPI_UNSIGNED_LONG_LONG, ROOT_RANK, 0, MPI_COMM_WORLD, &status);
        vec = arrayCreate(m);
        MPI_Recv(vec, m, MPI_DOUBLE, ROOT_RANK, 0, MPI_COMM_WORLD, &status);

        MPI_Recv(&n, 1, MPI_UNSIGNED_LONG_LONG, ROOT_RANK, 0, MPI_COMM_WORLD, &status);
        mat = arrayCreate(n * m);
        MPI_Recv(mat, n * m, MPI_DOUBLE, ROOT_RANK, 0, MPI_COMM_WORLD, &status);
        #endif

        root_n = n;
    }
    
    // Умножение полученных строк матрицы на вектор. Получение локального результат
    res = arrayCreate(root_n);
    matVecMult(res, mat, vec, n, m);

    if (rank == ROOT_RANK) {
        // Если процесс главный -- 
        // Принимаем локальные результаты других процессов
        // Добавляем результы к результату ROOT_RANK процесса
        size_t shift = n;
        for (uint8_t i = 1; i < size; ++i) {
            #ifdef MYMPI
            myMPIRecv(&n, 1, sizeof(size_t), i, 0);
            myMPIRecv(res + shift, n, sizeof(double), i, 0);
            #else
            MPI_Recv(&n, 1, MPI_UNSIGNED_LONG_LONG, i, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(res + shift, n, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
            #endif

            shift += n;
        }
    }
    else {
        // Если процесс не главный --
        // Посылаем локальный результат главному процессу 
        #ifdef MYMPI
        myMPISend(&n, 1, sizeof(size_t), ROOT_RANK, 0);
        myMPISend(res, n, sizeof(double), ROOT_RANK, 0);
        #else
        MPI_Send(&n, 1, MPI_UNSIGNED_LONG_LONG, ROOT_RANK, 0, MPI_COMM_WORLD);
        MPI_Send(res, n, MPI_DOUBLE, ROOT_RANK, 0, MPI_COMM_WORLD);
        #endif
    }
    
    // Ждём все процессы перед замером времени
    #ifdef MYMPI
    myMPIBarrier();
    #else 
    MPI_Barrier(MPI_COMM_WORLD);
    #endif

    clock_gettime(CLOCK_MONOTONIC, &stop);

    // Выводим результат работы
    if (rank == ROOT_RANK) {
        #ifdef RESULT
        printf("matrix:\n");
        for (size_t i = 0; i < root_n; ++i) {
            for (size_t j = 0; j < root_m; ++j)
                printf("%g ", matAccess(root_mat, root_m, i, j));
            printf("\n");
        }

        printf("vector:\n");
        for (size_t i = 0; i < m; ++i)
            printf("%g ", vec[i]);
        printf("\n");
        
        printf("result:\n");
        for (size_t i = 0; i < root_n; ++i)
            printf("%g ", res[i]);
        printf("\n");
        #endif

        printf("Elapsed time: %lf\n", clocktimeDifference(start, stop));
    }
    
    free(mat);
    free(vec);
    free(res);

    // Очищаем состояние MPI
    #ifdef MYMPI
    myMPIFinalize();
    #else 
    MPI_Finalize();
    #endif

    return 0;
}