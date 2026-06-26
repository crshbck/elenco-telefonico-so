#include "../utils.h"

#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 5050
#define MAX_CONNECTIONS 3

struct connection_args
{
	int client_addr;
	int conn_fd;
};

void *connection_handler(void *);

void ipaddrtstr(int ip, char *result)
{
	sprintf(result, "%d.%d.%d.%d", (ip) & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF,
			(ip >> 24) & 0xFF);

} // source: stackoverflow

sem_t conn_sem;

int main(int argc, char **argv)
{
	printf("Started server!\n");

	if (sem_init(&conn_sem, 0, MAX_CONNECTIONS) == -1)
	{
		error("Semaphore initialization error!");
	}

	int sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock_fd == -1)
	{
		error("Socket initialization error!");
	}

	struct sockaddr_in sock_addr = {0};
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(SERVER_PORT);
	sock_addr.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(sock_fd);
		error("Socket option set error!");
	}

	if (bind(sock_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1)
	{
		error("Socket bind error!");
	}

	if (listen(sock_fd, MAX_CONNECTIONS) == -1)
	{
		error("Socket listen error!");
	}

	struct sockaddr_in client_addr = {0};
	socklen_t client_addrlen = sizeof(client_addr);

	while (1)
	{
		int conn_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addrlen);

		// Accept connection
		if (sem_trywait(&conn_sem) == 0)
		{
			pthread_t thread;

			struct connection_args *args = malloc(sizeof(struct connection_args));
			args->client_addr = client_addr.sin_addr.s_addr;
			args->conn_fd = conn_fd;

			pthread_create(&thread, NULL, connection_handler, (void *) args);
		}
		// Refuse connection
		else
		{
			write(conn_fd, "Connection refused! Server Busy\n", 33); // TODO: cambia
			close(conn_fd);
		}
	}

	return 0;
}

void *connection_handler(void *args)
{
	int size = 256;

	struct connection_args *conn_args = (struct connection_args *) args;

	char ipstr[16];
	ipaddrtstr(conn_args->client_addr, ipstr);

	printf("Connected to %s!\n", ipstr);

	if (conn_args->conn_fd == -1)
	{
		error("Socket accept error!");
	}

	char *mess_buf = calloc(size + 1, sizeof(char));

	if (mess_buf == NULL)
	{
		error("Buffer allocation error!");
	}

	while (recv(conn_args->conn_fd, NULL, 0, MSG_PEEK | MSG_TRUNC) != -1 &&
		   read(conn_args->conn_fd, (void *) mess_buf, size) > 0)
	{
		// Add terminator incase client has not
		mess_buf[size] = '\0';

		printf("Recived: %s\n", mess_buf);

		memset(mess_buf, 0, size + 1);
	}

	close(conn_args->conn_fd);

	printf("Disconnected from %s!\n", ipstr);

	free(mess_buf);

	sem_post(&conn_sem);

	return NULL;
}
