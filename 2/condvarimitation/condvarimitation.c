#include "condvarimitation.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Узел очереди блокировок
// (Состоянние заблокировванного потока)
typedef struct CondNode {
    bool* state;

    struct CondNode* next;
} CondNode;

// Очередь блокировок
struct CondQueue {
    struct CondNode* head;
    struct CondNode* tail;
};

// Добавление состояние поток в очередь блокировок
static void condQueueSet(CondQueue* queue, bool* state) {
    CondNode* new_node = (CondNode*)malloc(sizeof(CondNode));
    if (new_node == NULL)
        throwErr("Error: new node out of memmory!");

    new_node->state = state;
    new_node->next = NULL;

    if (queue->head) {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    else 
        queue->head = queue->tail = new_node;
}

// Взятие состояние поток из очереди блокировок
static bool* condQueueGet(CondQueue* queue) {
    bool* state = NULL;

    if (queue->head) {
        state = queue->head->state;
        CondNode* delete_node = queue->head;

        if (queue->head != queue->tail)
            queue->head = queue->head->next;
        else
            queue->head = queue->tail = NULL;

        free(delete_node);
    }

    return state;
}

// Инициализация условной переменнной 
void condVarInit(CondVar* cond) {
    int err = pthread_mutex_init(&cond->mutex, NULL);
    if (err != 0)
        throwErr("Error: cannot initialize mutex!");

    cond->queue = (CondQueue*)malloc(sizeof(CondQueue));
    if (cond->queue == NULL)
        throwErr("Error: condition queue out of memmory!");

    cond->queue = (CondQueue*)malloc(sizeof(CondQueue));
}

// Блокировка до наступления события
void condVarWait(CondVar* cond, pthread_mutex_t* mutex) {
    // Блокируем условную переменную
    int err = pthread_mutex_lock(&cond->mutex);
    if (err != 0)
        throwErr("Error: cannot lock condition mutex!");

    // Сохраняем состояние потока
    bool* lock = (bool*)malloc(sizeof(bool));
    *lock = true;
    condQueueSet(cond->queue, lock);

    // Освобождаем условную переменную
    err = pthread_mutex_unlock(&cond->mutex);
    if (err != 0)
        throwErr("Error: cannot unlock condition mutex!");

    // Освобождаем мьютекс до получения сигнала
    err = pthread_mutex_unlock(mutex);
    if (err != 0)
        throwErr("Error: cannot unlock mutex!");

    // Ждём пока не получен сигнал
    while (*lock)
        usleep(WAIT_TIME_MS);
    free(lock);

    // Блокируем мьютекс после получения сигнала
    err = pthread_mutex_lock(mutex);
    if (err != 0)
        throwErr("Error: cannot lock mutex!");
}

// Сигнал для выхода из блокировки одного потока
void condVarSignal(CondVar* cond) {
    // Блокируем условную переменную
    int err = pthread_mutex_lock(&cond->mutex);
    if (err != 0)
        throwErr("Error: cannot lock condition mutex!");

    // Посылаем сигнал хотя бы одному потоку
    bool* lock = condQueueGet(cond->queue);
    if (lock)
        *lock = false;

    // Освобождаем условную переменную
    err = pthread_mutex_unlock(&cond->mutex);
    if (err != 0)
        throwErr("Error: cannot unlock condition mutex!");
}

// Сигнал для выхода из блокировки всех потоков
void condVarBroadcast(CondVar* cond) {
    // Блокируем условную переменную
    int err = pthread_mutex_lock(&cond->mutex);
    if (err != 0)
        throwErr("Error: cannot lock condition mutex!");

    // Посылаем сигнал всем потокам
    bool* lock = NULL;
    while ((lock = condQueueGet(cond->queue))) 
        *lock = false;

    // Освобождаем условную переменную
    err = pthread_mutex_unlock(&cond->mutex);
    if (err != 0)
        throwErr("Error: cannot unlock condition mutex!");
}

// Уничтожение условной переменнной
void condVarDestroy(CondVar* cond) {
    pthread_mutex_destroy(&cond->mutex);
    while (condQueueGet(cond->queue));
    free(cond->queue);
}