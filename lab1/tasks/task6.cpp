/* 6. Добавить в программу возможность передавать в поток сразу несколько параметров (см. пример 3). */

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <cerrno>
using namespace std;

#define ARGS_COUNT 3

// Структура для передачи нескольких параметров в поток
struct ThreadParams
{
	int thread_num;					// Номер потока
	char *args[ARGS_COUNT]; // Параметры функции main
};

// Количество потоков
int n = 5;
pthread_mutex_t mutex;

/* Функция для обработки ошибок */
void check_error(int err, const string &error_message)
{
	if (err != 0)
	{
		cout << error_message << ": " << strerror(err) << endl;
		exit(-1);
	}
}

/* Функция, которую будет исполнять созданный поток */
void *thread_job(void *arg)
{
	ThreadParams *params = (ThreadParams *)arg;
	int thread_num = params->thread_num;			 // Получаем номер потока
	pthread_t current_thread = pthread_self(); // Получаем текущий поток

	pthread_attr_t current_thread_attr;
	int err = pthread_getattr_np(current_thread, &current_thread_attr);
	check_error(err, "Getting current attribute failed");

	size_t current_stack_size;
	err = pthread_attr_getstacksize(&current_thread_attr, &current_stack_size);
	check_error(err, "Getting stack_size failed");

	size_t current_guard_size;
	err = pthread_attr_getguardsize(&current_thread_attr, &current_guard_size);
	check_error(err, "Getting guard_size failed");

	err = pthread_mutex_lock(&mutex);
	check_error(err, "Locking mutex failed");

	cout << "#############################" << endl;
	cout << "Thread " << thread_num << " is running..." << endl;
	cout << "Thread " << thread_num << " stack_size : " << current_stack_size << endl;
	cout << "Thread " << thread_num << " guard_size : " << current_guard_size << endl;
	cout << "Thread " << thread_num << " Params:" << endl;
	for (int i = 0; i < ARGS_COUNT; i++)
	{
		cout << params->args[i] << " ";
	}
	cout << endl;
	cout << "#############################" << endl;

	err = pthread_mutex_unlock(&mutex);
	check_error(err, "Unlocking mutex failed");
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != ARGS_COUNT)
	{
		check_error(EINVAL, "Wrong number of arguments");
	}

	cout << "Введите количество потоков: ";
	cin >> n;

	pthread_t threads[n];					 // Массив идентификаторов потоков
	int thread_args[n];						 // Аргументы для каждого потока
	int err;											 // Код ошибки
	pthread_attr_t thread_attr;		 // Атрибут потока
	ThreadParams thread_params[n]; // Параметры для каждого потока

	// Создаём n потоков
	for (int i = 0; i < n; ++i)
	{
		thread_args[i] = i; // Установка номера потока
		thread_params[i].thread_num = i;
		for (int j = 0; j < ARGS_COUNT; j++)
		{
			thread_params[i].args[j] = argv[j];
		}
		err = pthread_attr_init(&thread_attr); // Создание атрибута
		check_error(err, "Cannot create thread attribute");

		int stack_size = (i + 1) * 1024 * 1024;
		err = pthread_attr_setstacksize(&thread_attr, stack_size); // Установка размера стека
		check_error(err, "Setting stack size attribute failed");

		size_t guard_size = (i + 5) * 1024;
		err = pthread_attr_setguardsize(&thread_attr, guard_size); // Установка размера защиты стека
		check_error(err, "Setting guard size attribute failed");

		err = pthread_create(&threads[i], &thread_attr, thread_job, &thread_params[i]);
		check_error(err, "Cannot create a thread");
	}

	// Освобождаем память, занятую под хранение атрибутов потока
	pthread_attr_destroy(&thread_attr);

	// Ожидаем завершения всех созданных потоков
	for (int i = 0; i < n; ++i)
	{
		pthread_join(threads[i], NULL);
	}

	cout << "All threads have finished execution." << endl;
	return 0;
}