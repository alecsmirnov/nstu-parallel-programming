#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pthread.h>

#define err_exit(code, str) {                                       \
    std::cerr << str << ": " << std::strerror(code) << std::endl;   \
    exit(EXIT_FAILURE);                                             \
}

const size_t OP_COUNT    = 100000;
const int    TASKS_COUNT = 10;

int task_list[TASKS_COUNT];
int current_task = 0;

#ifdef MUTEX
pthread_mutex_t mutex;
#endif

void do_task(int task_no) {
    for (size_t i = 0; i != (task_no + 1) * OP_COUNT; ++i);
}

void* thread_job(void* arg) {
    int task_no;
    int err;

    // Перебираем в цикле доступные задания
    while (true) {
        #ifdef MUTEX
        // Захватываем мьютекс для исключительного доступа
        // к указателю текущего задания (переменная current_task)
        err = pthread_mutex_lock(&mutex);
        if (err != 0)
            err_exit(err, "Cannot lock mutex");
        #endif

        // Запоминаем номер текущего задания, которое будем исполнять
        task_no = current_task;
        sleep(1);

        // Сдвигаем указатель текущего задания на следующее
        current_task++;

        #ifdef MUTEX
        // Освобождаем мьютекс
        err = pthread_mutex_unlock(&mutex);
        if (err != 0)
            err_exit(err, "Cannot unlock mutex");
        #endif

        // Если запомненный номер задания не превышает
        // количества заданий, вызываем функцию, которая выполнит задание.
        // В противном случае завершаем работу потока
        if (task_no < TASKS_COUNT) {
            std::cout << "Thread " << pthread_self() << " execute the task " << task_no << std::endl;
            do_task(task_no);
        }
        else
            return NULL;
    }
}

int main(int argc, char* argv[]) {
    pthread_t thread1, thread2;
    int err;

    #ifdef MUTEX
    // Инициализируем мьютекс
    err = pthread_mutex_init(&mutex, NULL);
    if (err != 0)
        err_exit(err, "Cannot initialize mutex");
    #endif

    // Создаём потоки
    err = pthread_create(&thread1, NULL, thread_job, NULL);
    if (err != 0)
        err_exit(err, "Cannot create thread 1");
    err = pthread_create(&thread2, NULL, thread_job, NULL);
    if (err != 0)
        err_exit(err, "Cannot create thread 2");
    
    // Дожидаемся завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    #ifdef MUTEX
    // Освобождаем ресурсы, связанные с мьютексом
    pthread_mutex_destroy(&mutex);
    #endif

    return 0;
}