#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>
#include <pthread.h>
#include <queue>

#define THREAD_POOL_SIZE 10 // Количество потоков в пуле

// HTTP-ответ
char response[] = "HTTP/1.1 200 OK\r\n"
									"Content-Type: text/html; charset=UTF-8\r\n\r\n"
									"<!DOCTYPE html><html><head><title>Bye-bye baby bye-bye</title>"
									"<style>body { background-color: #111 }"
									"h1 { font-size:4cm; text-align: center; color: black;"
									" text-shadow: 0 0 2mm red}</style></head>"
									"<body><h1>Goodbye, world!</h1></body></html>\r\n";

// Очередь клиентов
std::queue<int> clientQueue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

// Функция потока из пула
void *worker_thread(void *arg)
{
	while (1)
	{
		int client_fd;

		// Захват мьютекса
		pthread_mutex_lock(&queue_mutex);
		while (clientQueue.empty())
		{
			pthread_cond_wait(&queue_cond, &queue_mutex);
		}

		client_fd = clientQueue.front();
		clientQueue.pop();
		pthread_mutex_unlock(&queue_mutex);

		// Логирование
		printf("Handling request from client %d\n", client_fd);

		char buffer[1024];
		read(client_fd, buffer, sizeof(buffer) - 1);

		if (send(client_fd, response, sizeof(response) - 1, MSG_NOSIGNAL) == -1)
		{
			perror("Failed to send to client");
		}

		if (close(client_fd) == -1)
		{
			perror("Failed to close client socket");
		}
	}
	return NULL;
}

int main()
{
	int one = 1;
	struct sockaddr_in svr_addr, cli_addr;
	socklen_t sin_len = sizeof(cli_addr);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		err(1, "Can't open socket");

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

	int port = 8080;
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_addr.s_addr = INADDR_ANY;
	svr_addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) == -1)
	{
		close(sock);
		err(1, "Can't bind");
	}

	listen(sock, 100);
	printf("Server is listening on port %d...\n", port);

	// Создаем пул потоков
	pthread_t pool[THREAD_POOL_SIZE];
	for (int i = 0; i < THREAD_POOL_SIZE; ++i)
	{
		pthread_create(&pool[i], NULL, worker_thread, NULL);
	}

	while (1)
	{
		int client_fd = accept(sock, (struct sockaddr *)&cli_addr, &sin_len);
		if (client_fd == -1)
		{
			perror("Can't accept");
			continue;
		}

		printf("Got connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

		// Кладем клиента в очередь
		pthread_mutex_lock(&queue_mutex);
		clientQueue.push(client_fd);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_signal(&queue_cond); // Сигнал потоку, что есть работа
	}

	close(sock);
	return 0;
}
