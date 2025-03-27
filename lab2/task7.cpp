#include <iostream>
#include <vector>
#include <pthread.h>
#include <functional>

using namespace std;

pthread_mutex_t mutex;

struct ThreadData
{
	const vector<int> *input;
	vector<int> *output;
	int start;
	int end;
	function<int(int)> mapFunc;
};

struct ReduceData
{
	const vector<int> *input;
	int start;
	int end;
	function<int(int, int)> reduceFunc;
	int result;
};

void *mapThread(void *arg)
{
	ThreadData *data = (ThreadData *)arg;
	for (int i = data->start; i < data->end; ++i)
	{
		(*data->output)[i] = data->mapFunc((*data->input)[i]);
	}
	return nullptr;
}

void *reduceThread(void *arg)
{
	ReduceData *data = (ReduceData *)arg;
	data->result = (*data->input)[data->start];
	for (int i = data->start + 1; i < data->end; ++i)
	{
		data->result = data->reduceFunc(data->result, (*data->input)[i]);
	}
	return nullptr;
}

vector<int> parallelMap(const vector<int> &inputArray, function<int(int)> mapFunc, int numThreads)
{
	vector<int> outputArray(inputArray.size());
	pthread_t threads[numThreads];
	ThreadData threadData[numThreads];
	int chunkSize = inputArray.size() / numThreads;
	int remainder = inputArray.size() % numThreads;
	int start = 0;

	for (int i = 0; i < numThreads; ++i)
	{
		int end = start + chunkSize + (i < remainder ? 1 : 0);
		threadData[i] = {&inputArray, &outputArray, start, end, mapFunc};
		pthread_create(&threads[i], nullptr, mapThread, &threadData[i]);
		start = end;
	}
	for (int i = 0; i < numThreads; ++i)
	{
		pthread_join(threads[i], nullptr);
	}
	return outputArray;
}

int parallelReduce(const vector<int> &inputArray, function<int(int, int)> reduceFunc, int numThreads)
{
	if (inputArray.empty())
		return 0;
	pthread_t threads[numThreads];
	ReduceData threadData[numThreads];
	vector<int> partialResults(numThreads);
	int chunkSize = inputArray.size() / numThreads;
	int remainder = inputArray.size() % numThreads;
	int start = 0;

	for (int i = 0; i < numThreads; ++i)
	{
		int end = start + chunkSize + (i < remainder ? 1 : 0);
		threadData[i] = {&inputArray, start, end, reduceFunc, 0};
		pthread_create(&threads[i], nullptr, reduceThread, &threadData[i]);
		start = end;
	}
	for (int i = 0; i < numThreads; ++i)
	{
		pthread_join(threads[i], nullptr);
		partialResults[i] = threadData[i].result;
	}
	int finalResult = partialResults[0];
	for (int i = 1; i < numThreads; ++i)
	{
		finalResult = reduceFunc(finalResult, partialResults[i]);
	}
	return finalResult;
}

int main()
{
	int size, numThreads;
	cout << "Enter the size of the array: ";
	cin >> size;
	cout << "Enter the number of threads: ";
	cin >> numThreads;

	vector<int> inputArray(size);
	for (int i = 0; i < size; ++i)
	{
		inputArray[i] = i + 1;
	}

	auto mapFunc = [](int x)
	{ return x * x; };
	auto reduceFunc = [](int a, int b)
	{ return a + b; };

	vector<int> mappedArray = parallelMap(inputArray, mapFunc, numThreads);
	int reducedResult = parallelReduce(mappedArray, reduceFunc, numThreads);

	cout << "Mapped array: ";
	for (int val : mappedArray)
	{
		cout << val << " ";
	}
	cout << endl;

	cout << "Reduced result: " << reducedResult << endl;

	return 0;
}
