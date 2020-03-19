#include <stdio.h>
#include <omp.h>

#define A_SIZE 100

int main(int argc, char* argv[]) {
    int sum = 0;
    int a[A_SIZE], id, size;

    for(int i = 0; i < A_SIZE; i++)
        a[i] = i;

    #ifdef CRITICAL
        #pragma omp parallel private(id, size) 
    #else  
        #ifdef TEST
            #pragma omp parallel private(size) shared(id)
        #else 
            #pragma omp parallel private(id, size) reduction(+ : sum)
        #endif
    #endif
    { 
        // Начало параллельной области
        id = omp_get_thread_num();
        size = omp_get_num_threads();

        // Разделяем работу между потоками
        int integer_part = A_SIZE / size;

        int remainder = A_SIZE % size;
        int a_local_size = integer_part + ((id < remainder) ? 1 : 0);

        int start = integer_part * id + ((id < remainder) ? id : remainder);
        int end = start + a_local_size;

        // Каждый поток суммирует элементы
        // своей части массива
        #ifdef CRITICAL
            // Для демонстрации частичной суммы потока
            int private_sum = 0;
            for(int i = start; i < end; i++)
                private_sum += a[i];

            printf("Thread %d, partial sum = %d\n", id, private_sum);

            #pragma omp critical(sum)
            sum += private_sum;
        #else 
            for(int i = start; i < end; i++)
                sum += a[i];

            printf("Thread %d, partial sum = %d\n", id, sum);
        #endif
    }

    // Благодаря опции reduction сумма частичных
    // результатов добавлена к значению переменной
    // sum начального потока
    printf("Final sum = %d\n", sum);

    return 0;
}
