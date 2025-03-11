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

// HTTP-ответ
char response[] = "HTTP/1.1 200 OK\r\n"
									"Content-Type: text/html; charset=UTF-8\r\n\r\n"
									"<!DOCTYPE html><html><head><title>Bye-bye baby bye-bye</title>"
									"<style>body { background-color: #111 }"
									"h1 { font-size:4cm; text-align: center; color: black;"
									" text-shadow: 0 0 2mm red}</style></head>"
									"<body><h1>Goodbye, world!</h1></body></html>\r\n";

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

	// Логирование
	printf("Handling request from client %d\n", client_fd);

	char buffer[1024];
	read(client_fd, buffer, sizeof(buffer) - 1);

	// Отправляем HTTP-ответ клиенту
	if (send(client_fd, response, sizeof(response) - 1, MSG_NOSIGNAL) == -1)
	{
		perror("Failed to send to client");
	}

	// Закрываем соединение
	// if (shutdown(client_fd, SHUT_WR) == -1)
	// {
	// 	perror("Failed to shutdown clien socket");
	// }
	if (close(client_fd) == -1)
	{
		perror("Failed to close client socket");
	}

	// Освобождаем память, выделенную для данных потока
	free(data);

	return NULL;
}

int main()
{
	int one = 1;
	struct sockaddr_in svr_addr, cli_addr;
	socklen_t sin_len = sizeof(cli_addr);

	// Создаем сокет
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		err(1, "can't open socket");

	// Устанавливаем опцию для повторного использования адреса
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

	// Настраиваем адрес сервера
	int port = 8080;
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_addr.s_addr = INADDR_ANY;
	svr_addr.sin_port = htons(port);

	// Привязываем сокет к адресу
	if (bind(sock, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) == -1)
	{
		close(sock);
		err(1, "Can't bind");
	}

	// Начинаем слушать входящие соединения
	listen(sock, 100);
	printf("Server is listening on port %d...\n", port);

	while (1)
	{
		// Принимаем новое соединение
		int client_fd = accept(sock, (struct sockaddr *)&cli_addr, &sin_len);
		if (client_fd == -1)
		{
			perror("Can't accept");
			continue;
		}

		printf("Got connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

		// Создаем структуру для передачи данных в поток
		ThreadData *data = (ThreadData *)malloc(sizeof(ThreadData));
		if (!data)
		{
			perror("Failed to allocate memory");
			close(client_fd);
			continue;
		}
		data->client_fd = client_fd;

		// Создаем поток для обработки запроса
		pthread_t thread;
		if (pthread_create(&thread, NULL, handle_request, data) != 0)
		{
			perror("Failed to create thread");
			free(data);
			close(client_fd);
			continue;
		}

		// Отсоединяем поток, чтобы он завершился самостоятельно
		pthread_detach(thread);
	}

	// Закрываем сокет (эта строка никогда не выполнится в данном примере)
	close(sock);
	return 0;
}