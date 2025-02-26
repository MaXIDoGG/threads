#include <cstdlib>
#include <iostream>
#include <cstring>
#include <pthread.h>
using namespace std;

// Количество потоков
int n = 5;

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
	int thread_num = *(int *)arg; // Получаем номер потока
	cout << "Thread " << thread_num << " is running..." << endl;

	pthread_t current_thread = pthread_self();

	pthread_attr_t current_thread_attr;
	int err = pthread_getattr_np(current_thread, &current_thread_attr);
	check_error(err, "Getting current attribute failed");

	size_t current_stack_size;
	err = pthread_attr_getstacksize(&current_thread_attr, &current_stack_size);
	check_error(err, "Getting stack_size failed");
	cout << "Current stack_size:" << current_stack_size << endl;

	size_t current_guard_size;
	err = pthread_attr_getguardsize(&current_thread_attr, &current_guard_size);
	check_error(err, "Getting guard_size failed");
	cout << "Current guard_size:" << current_guard_size << endl;
	return 0;
}

int main()
{
	cout << "Введите количество потоков: ";
	cin >> n;

	pthread_t threads[n];				// Массив идентификаторов потоков
	int thread_args[n];					// Аргументы для каждого потока
	int err;										// Код ошибки
	pthread_attr_t thread_attr; // Атрибут потока

	// Создаём n потоков
	for (int i = 0; i < n; ++i)
	{
		thread_args[i] = i;										 // Установка номера потока
		err = pthread_attr_init(&thread_attr); // Создание атрибута
		check_error(err, "Cannot create thread attribute");

		int stack_size = (i + 1) * 1024 * 1024;
		err = pthread_attr_setstacksize(&thread_attr, stack_size); // Установка размера стека
		check_error(err, "Setting stack size attribute failed");

		size_t guard_size = (i + 1) * 1024;
		err = pthread_attr_setguardsize(&thread_attr, guard_size); // Установка размера защиты стека
		check_error(err, "Setting guard size attribute failed");

		err = pthread_create(&threads[i], &thread_attr, thread_job, &thread_args[i]);
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