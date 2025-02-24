#include <cstdlib>
#include <iostream>
#include <cstring>
#include <pthread.h>
using namespace std;

// Количество потоков
const int n = 5;

/* Функция, которую будет исполнять созданный поток */
void *thread_job(void *arg)
{
	int thread_num = *(int *)arg; // Получаем номер потока
	cout << "Thread " << thread_num << " is running..." << endl;
	return 0;
}

int main()
{
	// Массив идентификаторов потоков
	pthread_t threads[n];
	int thread_args[n]; // Аргументы для каждого потока
	int err;

	// Создаём n потоков
	for (int i = 0; i < n; ++i)
	{
		thread_args[i] = i; // Устанавливаем номер потока
		err = pthread_create(&threads[i], NULL, thread_job, &thread_args[i]);
		// Если при создании потока произошла ошибка, выводим
		// сообщение об ошибке и прекращаем работу программы
		if (err != 0)
		{
			cout << "Cannot create a thread " << i << ": " << strerror(err) << endl;
			exit(-1);
		}
	}

	// Ожидаем завершения всех созданных потоков
	for (int i = 0; i < n; ++i)
	{
		pthread_join(threads[i], NULL);
	}

	cout << "All threads have finished execution." << endl;
	return 0;
}