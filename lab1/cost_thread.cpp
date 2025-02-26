#include <cstdlib>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Количество потоков
const int n = 5;

// Количество операций, которые будет выполнять каждый поток
const int operations_count = 1000000;

/* Функция, которую будет исполнять созданный поток */
void *thread_job(void *arg)
{
	int thread_num = *(int *)arg; // Получаем номер потока
	cout << "Thread " << thread_num << " is running..." << endl;

	// Выполняем арифметические операции
	int result = 0;
	for (int i = 0; i < operations_count; ++i)
	{
		result += i * i; // Простая арифметическая операция
	}

	cout << "Thread " << thread_num << " finished with result: " << result << endl;
	return 0;
}

int main()
{
	// Массив идентификаторов потоков
	pthread_t threads[n];
	int thread_args[n]; // Аргументы для каждого потока
	int err;

	// Замеряем время создания и завершения потоков
	auto start = high_resolution_clock::now();

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

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(end - start);

	cout << "All threads have finished execution." << endl;
	cout << "Total time taken: " << duration.count() << " microseconds" << endl;

	return 0;
}