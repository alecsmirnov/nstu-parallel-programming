#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <pthread.h>

// Параметры потока
struct ThreadParam {
	std::uint8_t num;	// Номер потока
	std::string msg;	// Сообщение потока
};

// Функция отлова ошибок
static void errorHandle(int test_code, int pass_code, std::string err_msg) {
	if (test_code != pass_code) {
		std::cerr << err_msg << std::strerror(test_code) << std::endl;
		exit(EXIT_FAILURE);
	}
}

// Вывод информации о созданном потоке
static void displayThreadInfo() {
	pthread_attr_t thread_attr;
	int err = pthread_getattr_np(pthread_self(), &thread_attr);
	errorHandle(err, 0, "Get thread attr error");

	std::cout << "Id: " << pthread_self() << std::endl;

	// Тип потока: присоединяемый или отсоединяемый
	int attr;
	err = pthread_attr_getdetachstate(&thread_attr, &attr);
	errorHandle(err, 0, "Get detach state error");
	std::cout << "Detach state: " << (attr == PTHREAD_CREATE_DETACHED ? "PTHREAD_CREATE_DETACHED" : "PTHREAD_CREATE_JOINABLE") << std::endl;

	// Ограничение потока: присоединён поток к процессу или нет
	err = pthread_attr_getscope(&thread_attr, &attr);
	errorHandle(err, 0, "Get scope state error");
	std::cout << "Scope: " << (attr == PTHREAD_SCOPE_PROCESS ? "PTHREAD_SCOPE_PROCESS" : "PTHREAD_SCOPE_SYSTEM") << std::endl;

	// Планирование потока: наследование дисциплины диспетчеризации из родительского процесса
	err = pthread_attr_getinheritsched(&thread_attr, &attr);
	errorHandle(err, 0, "Get inheritance state error");
	std::cout << "Inherit scheduler: " << (attr == PTHREAD_INHERIT_SCHED ? "PTHREAD_INHERIT_SCHED" : "PTHREAD_EXPLICIT_SCHED") << std::endl;

	// Приоритет потока
	struct sched_param sched;
	err = pthread_attr_getschedparam(&thread_attr, &sched);
	errorHandle(err, 0, "Get shedule param state error");
	std::cout << "Scheduling priority: " << sched.__sched_priority << std::endl;

	// Стек потока: адрес, размер
	void* stack_addr;
	std::size_t stack_size;
	err = pthread_attr_getstack(&thread_attr, &stack_addr, &stack_size);
	errorHandle(err, 0, "Get stack error");
	std::cout << "Stack addr: " << stack_addr << std::endl;
	std::cout << "Stack size: " << stack_size << " bytes\n" << std::endl;
}

// Функция, которую будет исполнять созданный поток
static void* threadJob(void* arg) {
	auto thread_param = *reinterpret_cast<ThreadParam*>(arg);

	sleep(thread_param.num);

	std::cout << "Thread " << +thread_param.num << " is running..." << std::endl;
	std::cout << "Message: " << thread_param.msg << std::endl;

	displayThreadInfo();

	pthread_exit(NULL);
}

// Создание потоков
void createThreads(std::uint8_t threads_count) {
	// Определяем переменные: информация о потоке (идентификатор потока и номер) и код ошибки
	std::vector<pthread_t> threads(threads_count);
	std::vector<ThreadParam> threads_param(threads_count);
	int err;

	pthread_attr_t thread_attr;

	// Создаём потоки
	for (std::uint8_t i = 0; i != threads.size(); ++i) {
		threads_param[i] = {i, "Hello from Thread " + std::to_string(i)};

		int err = pthread_attr_init(&thread_attr);
		errorHandle(err, 0, "Cannot create thread attribute: ");

		err = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
		errorHandle(err, 0, "Setting thread type failed: ");

		err = pthread_attr_setstacksize(&thread_attr, (i + 1) * 1024 * 1024);
		errorHandle(err, 0, "Setting thread stack size failed: ");

		err = pthread_create(&threads[i], &thread_attr, threadJob, reinterpret_cast<void*>(&threads_param[i]));
		errorHandle(err, 0, "Cannot create a thread: ");
	}

	// Ожидаем завершения созданных потоков перед завершением работы программы
	for (std::uint8_t i = 0; i != threads.size(); ++i) {
		err = pthread_join(threads[i], NULL);
		errorHandle(err, 0, "Cannot join a thread");
	}

	pthread_attr_destroy(&thread_attr);
	std::vector<ThreadParam>().swap(threads_param);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Wrong number of arguments!\n" << "Enter threads count\n" << std::endl;
		exit(EXIT_FAILURE);
	}

	createThreads(std::stoi(argv[1]));

	return 0;
}
