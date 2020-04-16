#include "mympi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* myMPI */
#define ARGS_COUNT 5
#define ROOT_RANK  0

/* Server */
#define EMIT_SIZE    10000
#define CLIENT_COUNT 128        // Размер очереди подключений

#define SOCK_ERR -1             // Код ошибки 
#define SOCK_PASS 0             // Код успеха

/* Функция отлова ошибок */
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

/* Тип отправленных/принятых данных */
enum DataType {
    DT_MSG,                     // Сообщение
    DT_BLOCK                    // Блокировка
};

/* Заголовок отправленных/принятых данных */
typedef struct DataHeader {
    int src;                    // Номер процесса отправителя
    int tag;

    uint8_t type;               // Тип заголовка
} DataHeader;

/* Данные серверной части процесса */
struct MyMPIData {
    int* send_socks;
    int* recv_socks;

    int lock_size;              // ROOT_RANK данные:
};                              // количество блокировок

/* Данные процесса */
struct MyMPIComm {
    const char* hostname;       // Адрес
    uint16_t port;              // Порт

    int rank;                   // Номер процесса
    int size;                   // Кол-во процессов
};

/* Отправка данных на сокет */
static void dataSend(int sock, void* data, size_t data_size) {
    if (data_size) {
        const char* data_ptr = (const char*)data;
        
        // Проверка данных на целостность
        size_t send_size = 0;
        while (send_size < data_size) {
            int bytes_send = send(sock, data_ptr, data_size - send_size, 0);
            if (bytes_send == SOCK_ERR)
                throwErr("Error: send data to dest!"); 

            data_ptr += bytes_send;
            send_size += bytes_send;
        }
    }
}

/* Приём данных на сокет */
static void dataRecv(int sock, void* data, size_t data_size) {
    if (data_size) {
        char* data_ptr = (char*)data;

        // Проверка данных на целостность
        size_t recv_size = 0;
        while (recv_size < data_size) {
            int bytes_recv = recv(sock, data_ptr, data_size - recv_size, 0);
            if (bytes_recv == SOCK_ERR)
                throwErr("Error: recv data from src!");

            data_ptr += bytes_recv;
            recv_size += bytes_recv;
        }
    }
}

/* Отправка больших данных на сокет */
static void largeDataSend(int sock, void* data, size_t data_size) {
    char* data_ptr = (char*)data;

    if (EMIT_SIZE < data_size) {
        size_t i;
        for (i = 0; i < data_size - EMIT_SIZE; i += EMIT_SIZE)
            dataSend(sock, data_ptr + i, EMIT_SIZE);

        data_ptr = data_ptr + i;
        data_size = data_size % EMIT_SIZE;;
    }

    dataSend(sock, data_ptr, data_size);
}

/* Приём больших данных на сокет */
static void largeDataRecv(int sock, void* data, size_t data_size) {
    char* data_ptr = (char*)data;

    if (EMIT_SIZE < data_size) {
        size_t i;
        for (i = 0; i < data_size - EMIT_SIZE; i += EMIT_SIZE)
            dataRecv(sock, data_ptr + i, EMIT_SIZE);

        data_ptr = data_ptr + i;
        data_size = data_size % EMIT_SIZE;;
    }

    dataRecv(sock, data_ptr, data_size);
}

/* Инициализация сокетов для отправки сообщений */
static void sendSocksInit() {
    mympi_data->send_socks = (int*)malloc(sizeof(int) * mympi_comm->size);
    if (mympi_data->send_socks == NULL)
        throwErr("Error: send socks out of memmory!");

    for (uint8_t i = 0; i != mympi_comm->size; ++i) {
        mympi_data->send_socks[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (mympi_data->send_socks[i] == SOCK_ERR)
            throwErr("Error: send socket open faild!"); 

        struct hostent* hosten = gethostbyname(mympi_comm->hostname); 

        struct sockaddr_in send_addr;
        send_addr.sin_family = AF_INET;
        send_addr.sin_addr.s_addr = ((struct in_addr*)hosten->h_addr_list[0])->s_addr;
        send_addr.sin_port = htons(mympi_comm->port + i);

        if (i == mympi_comm->rank) {
            int err = 0;
            int on = 1;

            err = setsockopt(mympi_data->send_socks[i], SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (int*)&on, sizeof(int));
            if (err == SOCK_ERR)
                throwErr("Error: setsockopt failed!");

            err = bind(mympi_data->send_socks[i], (struct sockaddr*)&send_addr, sizeof(send_addr));
            if (err == SOCK_ERR)
                throwErr("Error: bind serv socket!");

            err = listen(mympi_data->send_socks[i], CLIENT_COUNT); 
            if (err == SOCK_ERR)
                throwErr("Error: listen serv socket!");
        }
        else {
            while (connect(mympi_data->send_socks[i], (struct sockaddr*)&send_addr, sizeof(send_addr)) != SOCK_PASS);

            dataSend(mympi_data->send_socks[i], (void*)&mympi_comm->rank, sizeof(int));
        }
    }
}

/* Инициализация сокетов для приёма */
static void recvSocksInit() {
    mympi_data->recv_socks = (int*)malloc(sizeof(int) * mympi_comm->size);
    if (mympi_data->recv_socks == NULL)
        throwErr("Error: recv socks out of memmory!");

    for (uint8_t i = 0; i != mympi_comm->size - 1; ++i) {
        struct sockaddr_in recv_addr;
        socklen_t recv_len = sizeof(recv_addr);

        int recv_sock = 0;
        do {
            recv_sock = accept(mympi_data->send_socks[mympi_comm->rank], (struct sockaddr*)&recv_addr, (socklen_t*)&recv_len);
            if (recv_sock < SOCK_ERR) 
                throwErr("Error: recv sock accept!"); 
        } while (recv_sock == SOCK_ERR);

        int recv_rank;
        dataRecv(recv_sock, (void*)&recv_rank, sizeof(int));

        mympi_data->recv_socks[recv_rank] = recv_sock; 
    }
}

/* Инициализация работы myMPI */
void myMPIInit(int* argc, char** argv[]) {
    if (*argc < ARGS_COUNT) 
        throwErr("Wrong number of arguments!\n"
                 "Enter: <hostname> <port> <rank> <number of processes>\n");

    mympi_data = (struct MyMPIData*)malloc(sizeof(struct MyMPIData));
    if (mympi_data == NULL)
        throwErr("Error: mympi_data out of memmory!");
    mympi_comm = (struct MyMPIComm*)malloc(sizeof(struct MyMPIComm));
    if (mympi_comm == NULL)
        throwErr("Error: mympi_comm out of memmory!");

    // Инициализация данных процесса
    mympi_comm->hostname =  (*argv)[1];
    mympi_comm->port = atoi((*argv)[2]);
    mympi_comm->rank = atoi((*argv)[3]);
    mympi_comm->size = atoi((*argv)[4]);

    // Очиста myMPI аргументов командой строки (сдвиг аргументов программы на их место)
    (*argc) -= ARGS_COUNT - 1;
    for (uint8_t i = 1; i != *argc; ++i)
        (*argv)[i] = (*argv)[i + ARGS_COUNT - 1];

    // Инициализация данных серверной части
    mympi_data->lock_size = 0;

    sendSocksInit();
    recvSocksInit();

    myMPIBarrier();
}

/* Отправка статуса блокировки указанному процессу */
static void lockSend(int dest) {
    if (dest == mympi_comm->rank)
        throwErr("Error: send lock to yourself!"); 

    // Посылаем блокирующее сообщение
    DataHeader data_header = (DataHeader){-1, -1, DT_BLOCK};
    dataSend(mympi_data->send_socks[dest], (void*)&data_header, sizeof(DataHeader));
}

/* Приём блокировок от других процессов */
static void lockRecv(int src) {
    if (src == mympi_comm->rank)
        throwErr("Error: recv lock from yourself!"); 

    DataHeader data_header;

    // Принимаем данные, пока не поступит блокирующее сообщение
    do {
        dataRecv(mympi_data->recv_socks[src], (void*)&data_header, sizeof(DataHeader));
    } while (data_header.type != DT_BLOCK);
}

/* Добавить блокировку (увеличить общее количество блокировок) */
static void lockAdd() {
    ++mympi_data->lock_size;
}

/* Очистить блокировки */
static void lockClear() {
    mympi_data->lock_size = 0;
}

/* Барьерная синхронизация потоков */
void myMPIBarrier() {
    // Если поток основной
    if (mympi_comm->rank == ROOT_RANK) {
        // Принимаем блокировки от других потоков,
        // пока все, другие потоки, не будут заблокированы
        for (uint8_t i = 0; i != mympi_comm->size; ++i)
            if (i != ROOT_RANK) {
                lockRecv(i);
                lockAdd();
            }
        
        // Когда все потоки заблокированы -- разблокируем их
        for (uint8_t i = 0; i != mympi_comm->size; ++i)
            if (i != ROOT_RANK) 
                lockSend(i);
    }
    else {
        // Если поток не основной --
        // посылаем сообщение о блокировке основному потоку 
        lockSend(ROOT_RANK);
        // Ждём сигнал для разблокировки от основного потока
        lockRecv(ROOT_RANK);
    }

    // Очищаем блокировки
    lockClear();
}

/* Завершение работы myMPI */
void myMPIFinalize() {
    myMPIBarrier();

    for (uint8_t i = 0; i != mympi_comm->size; ++i) {
        close(mympi_data->send_socks[i]);
        close(mympi_data->recv_socks[i]);
    }

    free(mympi_data->send_socks);
    free(mympi_data->recv_socks);

    free(mympi_data);
    free(mympi_comm);
}

/* Получить номер процесса */
void myMPICommRank(int* rank) {
    if (rank == NULL) 
        throwErr("Error: rank null ptr!"); 

    *rank = mympi_comm->rank;
}

/* Получить общее число процессов */
void myMPICommSize(int* size) {
    if (size == NULL)
        throwErr("Error: size null ptr!"); 

    *size = mympi_comm->size;
}

/* Получить порт */
void myMPICommPort(uint16_t* port) {
    if (port == NULL)
        throwErr("Error: port null ptr!"); 

    *port = mympi_comm->port + mympi_comm->rank;
}

/* Отправка данных процессу */
void myMPISend(void* data, size_t count, size_t data_type, int dest, int tag) {
    if (dest == mympi_comm->rank)
        throwErr("Error: recv data from yourself!"); 

    DataHeader data_header = (DataHeader){mympi_comm->rank, tag, DT_MSG};

    dataSend(mympi_data->send_socks[dest], (void*)&data_header, sizeof(DataHeader));
    largeDataSend(mympi_data->send_socks[dest], data, count * data_type);
}

/* Приём данных процесса */
void myMPIRecv(void* data, size_t count, size_t data_type, int src, int tag) {
    if (src == mympi_comm->rank)
        throwErr("Error: recv data from yourself!"); 
    
    DataHeader data_header;

    // Ожидаем сообщение, пока номер и тэг сообщения не соответствует указанным
    do {
        dataRecv(mympi_data->recv_socks[src], (void*)&data_header, sizeof(DataHeader));

        // Проверяем тип сообщения. Если получена блокировка -- запоминаем её, для будущей проверки
        switch (data_header.type) {
            case DT_MSG:   largeDataRecv(mympi_data->recv_socks[src], data, count * data_type); break;
            case DT_BLOCK: lockAdd();                                                           break;
            default:       throwErr("Error: unrecognized data type!");
        }
    } while (data_header.src != src || data_header.tag != tag);
}