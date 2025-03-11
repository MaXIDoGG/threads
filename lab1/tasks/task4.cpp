#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
using namespace std;

// Количество операций, которые выполняет поток
#define OPERATIONS 1000000

// Функция, которую выполняет поток
void *thread_function(void *arg)
{
	int count = *((int *)arg);
	int summa = 0;
	for (int i = 0; i < count; i++)
	{
		summa += 1;
	}
	return NULL;
}

// Функция для измерения времени
double get_time()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main()
{
	pthread_t thread;
	int count;
	cout << "Enter the number of operations: ";
	cin >> count;
	double start_time, end_time;

	// Измеряем время выполнения операций в одном потоке
	start_time = get_time();
	thread_function(&count);
	end_time = get_time();
	double single_thread_time = end_time - start_time;
	printf("Single-threaded execution time: %.6f seconds\n", single_thread_time);

	// Измеряем время создания и завершения потока
	start_time = get_time();
	pthread_create(&thread, NULL, thread_function, &count);
	pthread_join(thread, NULL);
	end_time = get_time();
	double thread_creation_time = end_time - start_time - single_thread_time;
	printf("Thread creation and destruction time: %.6f seconds\n", thread_creation_time);

	return 0;
}