#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int ready = 0;

void *producer(void *arg)
{
	sleep(2); // Имитация работы
	pthread_mutex_lock(&mutex);
	ready = 1;
	printf("Producer: данные готовы!\n");
	pthread_mutex_unlock(&mutex);
	return NULL;
}

void *consumer(void *arg)
{
	pthread_mutex_lock(&mutex);
	while (ready == 0)
	{
		pthread_mutex_unlock(&mutex);
		usleep(100000); // Ожидание перед повторной проверкой
		pthread_mutex_lock(&mutex);
	}
	printf("Consumer: получил данные!\n");
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main()
{
	pthread_t t1, t2;
	pthread_create(&t1, NULL, consumer, NULL);
	pthread_create(&t2, NULL, producer, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	pthread_mutex_destroy(&mutex);
	return 0;
}
