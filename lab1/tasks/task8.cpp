/* 8. Разработать программу, которая обеспечивает параллельное
применение заданной функции к каждому элементу массива. Размер
массива, применяемая функция и количество потоков задаются динамически*/

#include <iostream>
#include <vector>
#include <pthread.h>
#include <cstring>
#include <functional>

using namespace std;

// Мьютекс для синхронизации вывода
pthread_mutex_t mutex;

// Структура для передачи данных в поток
struct ThreadData
{
	const vector<int> *input; // Указатель на входной массив
	vector<int> *output;			// Указатель на выходной массив
	int start;								// Начальный индекс для обработки
	int end;									// Конечный индекс для обработки
	function<int(int)> func;	// Функция для применения к элементам
};

// Функция для применения к каждому элементу массива
int square(int x)
{
	return x * x;
}

int cube(int x)
{
	return x * x * x;
}

// Функция, которую выполняет каждый поток
void *processChunk(void *arg)
{
	ThreadData *data = (ThreadData *)arg;

	for (int i = data->start; i < data->end; ++i)
	{
		(*data->output)[i] = data->func((*data->input)[i]); // Применяем функцию к элементу массива
	}

	// Синхронизированный вывод информации о потоке
	pthread_mutex_lock(&mutex);
	cout << "Thread " << pthread_self() << " processed elements from " << data->start << " to " << data->end - 1 << endl;
	pthread_mutex_unlock(&mutex);

	return nullptr;
}

int main()
{
	int size, numThreads;

	// Ввод размера массива
	cout << "Enter the size of the array: ";
	cin >> size;

	// Ввод количества потоков
	cout << "Enter the number of threads: ";
	cin >> numThreads;

	// Выбор функции
	function<int(int)> func;
	cout << "Choose a function (1: square, 2: cube): ";
	int choice;
	cin >> choice;
	if (choice == 1)
	{
		func = square;
	}
	else if (choice == 2)
	{
		func = cube;
	}
	else
	{
		cout << "Invalid choice. Using square function by default." << endl;
		func = square;
	}

	// Создание и заполнение массива
	vector<int> inputArray(size);
	for (int i = 0; i < size; ++i)
	{
		inputArray[i] = i + 1; // Заполняем массив значениями 1, 2, 3, ..., size
	}

	// Вектор для результатов
	vector<int> outputArray(size);

	// Создание потоков
	pthread_t threads[numThreads];
	ThreadData threadData[numThreads];

	int chunkSize = size / numThreads; // Размер части массива для каждого потока
	int remaining = size % numThreads; // Остаток, если размер массива не делится нацело на количество потоков

	int start = 0;
	for (int i = 0; i < numThreads; ++i)
	{
		int end = start + chunkSize + (i < remaining ? 1 : 0); // Распределяем остаток по потокам

		// Заполняем структуру данных для потока
		threadData[i] = {&inputArray, &outputArray, start, end, func};

		// Создаем поток
		int err = pthread_create(&threads[i], nullptr, processChunk, &threadData[i]);
		if (err != 0)
		{
			cerr << "Failed to create thread: " << strerror(err) << endl;
			return 1;
		}

		start = end;
	}

	// Ожидание завершения всех потоков
	for (int i = 0; i < numThreads; ++i)
	{
		pthread_join(threads[i], nullptr);
	}

	// Вывод результатов
	cout << "Input array: ";
	for (int val : inputArray)
	{
		cout << val << " ";
	}
	cout << endl;

	cout << "Output array: ";
	for (int val : outputArray)
	{
		cout << val << " ";
	}
	cout << endl;

	// Уничтожение мьютекса
	pthread_mutex_destroy(&mutex);

	return 0;
}