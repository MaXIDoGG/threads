#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx; // Мьютекс для синхронизации вывода

void print_numbers(int id)
{
	std::lock_guard<std::mutex> lock(mtx); // Блокируем мьютекс
	for (int i = 0; i < 10; ++i)
	{
		std::cout << "Thread " << id << ": " << i << std::endl;
	}
}

int main()
{
	std::vector<std::thread> threads;

	// Создаем несколько потоков
	for (int i = 0; i < 5; ++i)
	{
		threads.emplace_back(print_numbers, i);
	}

	// Ожидаем завершения всех потоков
	for (auto &t : threads)
	{
		t.join();
	}

	return 0;
}