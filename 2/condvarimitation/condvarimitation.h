#ifndef CONDVARIMITATION_H
#define CONDVARIMITATION_H

#include <pthread.h>

// Время ожидания, между проверками сигнала
// для разблокировки переменной
#define WAIT_TIME_MS 1E+6

// Очередь блокировок
typedef struct CondQueue CondQueue;

// Тип условной переменной
typedef struct CondVar {
    pthread_mutex_t mutex;

    struct CondQueue* queue;
} CondVar;

// Инициализация условной переменнной 
void condVarInit(CondVar* cond);

// Блокировка до наступления события
void condVarWait(CondVar* cond, pthread_mutex_t* mutex);

// Сигнал для выхода из блокировки хотя бы одного потока
void condVarSignal(CondVar* cond);
// Сигнал для выхода из блокировки всех потоков
void condVarBroadcast(CondVar* cond);

// Уничтожение условной переменнной 
void condVarDestroy(CondVar* cond);

#endif