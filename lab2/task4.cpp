#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM_ITERATIONS 1000000
#define MAX_THREADS 32

// Глобальные переменные
int shared_counter = 0;
pthread_mutex_t mutex;
pthread_spinlock_t spinlock;

// Тест с мьютексом
void *mutex_test(void *arg)
{
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		pthread_mutex_lock(&mutex);
		shared_counter++;
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

// Тест со спинлоком
void *spinlock_test(void *arg)
{
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		pthread_spin_lock(&spinlock);
		shared_counter++;
		pthread_spin_unlock(&spinlock);
	}
	return NULL;
}

double run_test(int num_threads, int use_spinlock)
{
	pthread_t threads[MAX_THREADS];
	struct timeval start, end;

	shared_counter = 0;
	gettimeofday(&start, NULL);

	for (int i = 0; i < num_threads; i++)
	{
		if (use_spinlock)
		{
			pthread_create(&threads[i], NULL, spinlock_test, NULL);
		}
		else
		{
			pthread_create(&threads[i], NULL, mutex_test, NULL);
		}
	}

	for (int i = 0; i < num_threads; i++)
	{
		pthread_join(threads[i], NULL);
	}

	gettimeofday(&end, NULL);
	return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

int main()
{
	pthread_mutex_init(&mutex, NULL);
	pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);

	printf("Threads\tMutex (s)\tSpinlock (s)\n");
	printf("--------------------------------------------\n");

	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
	{
		double mutex_time = run_test(num_threads, 0);
		double spinlock_time = run_test(num_threads, 1);
		printf("%d\t%.4f\t\t%.4f\n",
					 num_threads, mutex_time, spinlock_time);
	}

	pthread_mutex_destroy(&mutex);
	pthread_spin_destroy(&spinlock);
	return 0;
}