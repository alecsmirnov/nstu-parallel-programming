#include "poisson3d.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// Номер главного ранка
#define ROOT_RANK 0

// Минимальный размер сетки по координате D.x
#define DX_SIZE_MIN 3

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Функция доступа к элементу решётки
#define gridAccess(grid, D, i, j, k) \
    ((grid)[(i) * D.y * D.z + (j) * D.z + (k)])

// Искомая функция
#define phi(x, y, z) \
    ((x) * (x) + (y) * (y) + (z) * (z))

// Правая часть уравнения
#define rho(x, y, z) \
    (6 - ALPHA * phi(x, y, z))

#define max(x, y) \
    ((x) > (y) ? (x) : (y))

// Проверка краевых элементов
static bool isBoundary(Point D, size_t i, size_t j, size_t k) {
    return i == 0 || i == D.x - 1 || 
           j == 0 || j == D.y - 1 || 
           k == 0 || k == D.z - 1;
}

// Составление краевых условий 1-го рода
static void setBoundary(double* grid_data, Point D, DPoint h, DPoint p0) {
    for (size_t i = 0; i != D.x; ++i)
        for (size_t j = 0; j != D.y; ++j)
            for (size_t k = 0; k != D.z; ++k) {
                double x = 0;
                double y = 0;
                double z = 0;

                if (isBoundary(D, i, j, k)) {
                    x = p0.x + i * h.x;
                    y = p0.y + j * h.y;
                    z = p0.z + k * h.z;
                }

                gridAccess(grid_data, D, i, j, k) = phi(x, y, z);
            }
}

// Формула итерационного процесса Якоби
static double jacobi(double* grid_data, Point D, DPoint h, DPoint p0, 
                     size_t i, size_t j, size_t k) {
    double hx2 = h.x * h.x;
    double hy2 = h.y * h.y;
    double hz2 = h.z * h.z;

    double phix = (gridAccess(grid_data, D, i - 1, j, k) + 
                   gridAccess(grid_data, D, i + 1, j, k)) / hx2;
    double phiy = (gridAccess(grid_data, D, i, j - 1, k) + 
                   gridAccess(grid_data, D, i, j + 1, k)) / hy2;
    double phiz = (gridAccess(grid_data, D, i, j, k - 1) + 
                   gridAccess(grid_data, D, i, j, k + 1)) / hz2;

    double x = p0.x + i * h.x;
    double y = p0.y + j * h.y;
    double z = p0.z + k * h.z;

    return (phix + phiy + phiz - rho(x, y, z)) / 
           (2 / hx2 + 2 / hy2 + 2 / hz2 + ALPHA);
}

// Последовательное решение уравнения
static void sequentialSolution(Point D, Point N, DPoint p0, P3DResult* result) {
    DPoint h = (DPoint){1.0 * D.x / (N.x - 1), 
                        1.0 * D.y / (N.y - 1), 
                        1.0 * D.z / (N.z - 1)};

    // Создание сетки
    double* grid_data = (double*)malloc(sizeof(double) * D.x * D.y * D.z);
    if (grid_data == NULL) 
        throwErr("Error: grid data out of memmory!");

    // Инициализация сетки
    setBoundary(grid_data, D, h, p0);

    // Сетка последующих итераций
    double* new_data = (double*)malloc(sizeof(double) * D.x * D.y * D.z);
    if (new_data == NULL)
        throwErr("Error: new grid out of memmory!");

    memcpy(new_data, grid_data, sizeof(double) * D.x * D.y * D.z);

    double jacobi_result = 0;
    size_t iters = 0;
    do {
        jacobi_result = 0;

        // Вычисление функции в узлах сетки
        for (size_t i = 1; i != D.x - 1; ++i)
            for (size_t j = 1; j != D.y - 1; ++j)
                for (size_t k = 1; k != D.z - 1; ++k) {
                    gridAccess(new_data, D, i, j, k) = jacobi(grid_data, D, h, p0, i, j, k);

                    jacobi_result = max(jacobi_result, fabs(gridAccess(grid_data, D, i, j, k) - 
                                                            gridAccess(new_data, D, i, j, k)));
                }

        memcpy(grid_data, new_data, sizeof(double) * D.x * D.y * D.z);

        ++iters;
        // Проверка порога сходимости и максимального кол-ва итераций
    } while (EPS < jacobi_result && iters < ITERS_MAX);

    free(grid_data);
    free(new_data);

    *result = (P3DResult){jacobi_result, iters};
}

#ifdef WITH_MPI
// Отправка граничных элементов между процессами
static void sendBorders(int rank, int size, 
                        MPI_Request* down_send, MPI_Request* down_recv, 
                        MPI_Request* up_send, MPI_Request* up_recv,
                        double* data, int local_size, int borders_offset, size_t block_size) {
    // Отправка и приём верхних границ
    if (rank < size - 1) {
        MPI_Isend(data + (local_size + borders_offset - block_size), block_size, 
                  MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, down_send);
        MPI_Irecv(data + (local_size + borders_offset), block_size, 
                  MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, down_recv);
    }

    // Отправка и приём нижних границ
    if (0 < rank) {
        MPI_Isend(data + borders_offset, block_size, 
                  MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, up_send);
        MPI_Irecv(data, block_size, MPI_DOUBLE, rank - 1, 0, 
                  MPI_COMM_WORLD, up_recv);
    }
}

// Приём граничных элементов
static void recvBorders(int rank, int size,
                        MPI_Request* down_send, MPI_Request* down_recv, 
                        MPI_Request* up_send, MPI_Request* up_recv) {
    // Ожидание верхних границ
    if (rank < size - 1) {
        MPI_Wait(down_send, MPI_STATUSES_IGNORE);
        MPI_Wait(down_recv, MPI_STATUSES_IGNORE);
    }

    // Ожидание нижних границ
    if (0 < rank) {
        MPI_Wait(up_send, MPI_STATUSES_IGNORE);
        MPI_Wait(up_recv, MPI_STATUSES_IGNORE);
    }
}

// Обработка блока данных по оси D.x
static double processBlock(double* grid_data, double* new_data, Point D, DPoint h, DPoint p0,
                           size_t x_beg, size_t x_end, double current_result) {
    double result = current_result;

    // Вычисление функции в узлах блока
    for (size_t i = x_beg; i != x_end; ++i)
        for (size_t j = 1; j != D.y - 1; ++j)
            for (size_t k = 1; k != D.z - 1; ++k) {
                gridAccess(new_data, D, i, j, k) = jacobi(grid_data, D, h, p0, i, j, k);

                result = max(result, fabs(gridAccess(grid_data, D, i, j, k) - 
                                          gridAccess(new_data, D, i, j, k)));
            }
    
    return result;
}

// Обработка поля данных (блоков, не доходящих до границ)
static double processField(int rank, int size, double* data, double* new_data, 
                           int local_size, int borders, size_t block_size,
                           Point D, DPoint h, DPoint p0, double current_result) {
    double result = current_result;

    // Обработка блока 0-го процесса
    if (rank == 0) {
        size_t x_beg = 1;
        size_t x_end = (local_size + borders) / block_size - 2;
        result = processBlock(data, new_data, D, h, p0, x_beg, x_end, result);
    }

    // Обработка блока последнего процесса
    if (rank == size - 1) {
        size_t x_beg = 2;
        size_t x_end = (local_size + borders) / block_size - 1;
        result = processBlock(data, new_data, D, h, p0, x_beg, x_end, result);
    }

    // Обработка центральных блоков
    if (0 < rank && rank < size - 1) {
        size_t x_beg = 2;
        size_t x_end = (local_size + borders) / block_size - 2;
        result = processBlock(data, new_data, D, h, p0, x_beg, x_end, result);
    }

    return result;
}

// Обработка границ
static double processBorders(int rank, int size, double* data, double* new_data,
                             int local_size, int borders, size_t block_size,
                             Point D, DPoint h, DPoint p0, double current_result) {
    double result = current_result;

    size_t x_bottom = (local_size + borders) / block_size - 2;
    size_t x_upper = 1;

    // Обработка нижней границы
    if (rank < size - 1)
        result = processBlock(data, new_data, D, h, p0, x_bottom, x_bottom + 1, result);
    
    // Обработка верхней границы
    if (0 < rank)
        result = processBlock(data, new_data, D, h, p0, x_upper, x_upper + 1, result);

    return result;
}

// Параллельное решение уравнения
static void parallelSolution(Point D, Point N, DPoint p0, int rank, int size, P3DResult* result) {
    P3DResult root_result;

    double* grid_data = NULL;

    size_t block_size = D.y * D.z;
    DPoint h = (DPoint){1.0 * D.x / (N.x - 1), 
                        1.0 * D.y / (N.y - 1), 
                        1.0 * D.z / (N.z - 1)};

    // Инициализация поля данных (сетки)
    if (rank == ROOT_RANK) {
        grid_data = (double*)malloc(sizeof(double) * D.x * D.y * D.z);
        if (grid_data == NULL) 
            throwErr("Error: grid data out of memmory!");

        setBoundary(grid_data, D, h, p0);
    }

    // Массив размеров локальных данных
    int* local_sizes = (int*)malloc(sizeof(int) * size);
    if (local_sizes == NULL) 
        throwErr("Error: local sizes out of memmory!");
    
    // Массив размеров сдвигов локальных данных
    int* offsets = (int*)malloc(sizeof(int) * size);
    if (offsets == NULL) 
        throwErr("Error: offsets out of memmory!");

    int chunk_size = D.x / size;
    int remainder = D.x % size;

    // Определение размеров локальных данных
    int shift = 0;
    for (int i = 0; i != size; ++i) {
        local_sizes[i] = chunk_size + (i < remainder ? 1 : 0);

        offsets[i] = shift;                         // Сдвиг строки локальных данных
        shift += local_sizes[i];

        local_sizes[i] *= block_size;               // Получение размера локального блока
        offsets[i] *= block_size;                   // Получение сдвига блока данных
    }

    // Определение границ и сдвигов границ
    int borders = 2 * block_size;                   // Кол-во границ
    int borders_offset = block_size;                // Сдвиг границ

    if (rank == 0 || rank == size - 1) {
        borders = block_size;
        if (rank == 0)
            borders_offset = 0;
    }

    // Выделение памяти под блок данных и границы
    double* local_data = (double*)malloc(sizeof(double) * (local_sizes[rank] + borders));
    if (local_data == NULL)
        throwErr("Error: local data out of memmory!");

    // Разбиение поля данных на блоки разного размера и передача другим процессам
    MPI_Scatterv(grid_data, local_sizes, offsets, MPI_DOUBLE, local_data + borders_offset, 
                 local_sizes[rank], MPI_DOUBLE, ROOT_RANK, MPI_COMM_WORLD);

    // Сетка последующих итераций
    double* new_data = (double*)malloc(sizeof(double) * (local_sizes[rank] + borders));
    if (new_data == NULL)
        throwErr("Error: new grid out of memmory!");

    memcpy(new_data, local_data, sizeof(double) * (local_sizes[rank] + borders));

    double jacobi_result = 0;
    size_t iters = 0;
    do {
        jacobi_result = 0;

        MPI_Request down_send;
        MPI_Request down_recv;
        MPI_Request up_send;
        MPI_Request up_recv;

        // Посылаем границы другим процессам
        sendBorders(rank, size, &down_send, &down_recv, &up_send, &up_recv, 
                    local_data, local_sizes[rank], borders_offset, block_size);

        // Обрабатываем поле данных (без границ)
        jacobi_result = processField(rank, size, local_data, new_data, 
                                     local_sizes[rank], borders, block_size, 
                                     D, h, p0, jacobi_result);

        // Принимаем границы
        recvBorders(rank, size, &down_send, &down_recv, &up_send, &up_recv);

        // Обрабатываем границы
        processBorders(rank, size, local_data, new_data, 
                       local_sizes[rank], borders, block_size, 
                       D, h, p0, jacobi_result);

        // Обмениваемся результатми между процессами и находи max
        MPI_Allreduce(MPI_IN_PLACE, &jacobi_result, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        memcpy(local_data, new_data, sizeof(double) * (local_sizes[rank] + borders));
        
        ++iters;
        // Проверка порога сходимости и максимального кол-ва итераций
    } while (EPS < jacobi_result && iters < ITERS_MAX);
    
    free(new_data);
    free(local_sizes);
    free(offsets);

    if (rank == ROOT_RANK) {
        free(grid_data);

        *result = (P3DResult){jacobi_result, iters};
    }
}
#endif

void solveEquation(Point D, Point N, DPoint p0, P3DResult* result) {
    int rank = ROOT_RANK;
    int size = 1;
    
    #ifdef WITH_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    #endif

    if (size == 1 || D.x / size < DX_SIZE_MIN)
        sequentialSolution(D, N, p0, result);
    
    else {
        #ifdef WITH_MPI
        parallelSolution(D, N, p0, rank, size, result);
        #endif
    }
}