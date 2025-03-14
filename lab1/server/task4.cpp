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
#include <string.h>

// Структура для передачи данных в поток
typedef struct
{
	int client_fd; // Дескриптор клиентского сокета
} ThreadData;

// Функция для обработки запроса в отдельном потоке
void *handle_request(void *arg)
{
	ThreadData *data = (ThreadData *)arg;
	int client_fd = data->client_fd;
	free(data);

	// Логирование
	printf("Handling request from client %d\n", client_fd);

	char buffer[1024];
	read(client_fd, buffer, sizeof(buffer) - 1);

	// Получение версии PHP
	FILE *php_script = popen("php -r 'echo PHP_VERSION;'", "r");
	if (!php_script)
	{
		perror("Failed to run PHP command");
		close(client_fd);
		return NULL;
	}

	char php_output[128];
	fgets(php_output, sizeof(php_output), php_script);
	pclose(php_script);

	// Создание HTTP-ответа
	char response[1024];
	snprintf(response, sizeof(response),
					 "HTTP/1.1 200 OK\r\n"
					 "Content-Type: text/html; charset=UTF-8\r\n\r\n"
					 "<!DOCTYPE html><html><head><title>PHP Version</title>"
					 "<style>body { background-color: #111; color: white; text-align: center; }</style></head>"
					 "<body><h1>Goodbye, world!</h1><p>PHP Version: %s</p></body></html>\r\n",
					 php_output);

	// Отправляем HTTP-ответ клиенту
	if (send(client_fd, response, strlen(response), MSG_NOSIGNAL) == -1)
	{
		perror("Failed to send to client");
	}

	close(client_fd);
	return NULL;
}

int main()
{
	pid_t pid = getpid();
	printf("pid: %u\n", pid);

	int one = 1;
	struct sockaddr_in svr_addr, cli_addr;
	socklen_t sin_len = sizeof(cli_addr);

	// Создаем сокет
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		err(1, "can't open socket");

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

	while (1)
	{
		int client_fd = accept(sock, (struct sockaddr *)&cli_addr, &sin_len);
		if (client_fd == -1)
		{
			perror("Can't accept");
			continue;
		}

		printf("Got connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

		ThreadData *data = (ThreadData *)malloc(sizeof(ThreadData));
		if (!data)
		{
			perror("Failed to allocate memory");
			close(client_fd);
			continue;
		}
		data->client_fd = client_fd;

		pthread_t thread;
		if (pthread_create(&thread, NULL, handle_request, data) != 0)
		{
			perror("Failed to create thread");
			free(data);
			close(client_fd);
			continue;
		}

		pthread_detach(thread);
	}

	close(sock);
	return 0;
}
