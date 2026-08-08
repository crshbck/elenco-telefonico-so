#include "../common/utils.h"
#include "../protocol.h"
#include "login.h"
#include "sender.h"

#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 5050
#define MAX_CONNECTIONS 3

typedef struct _connection_args
{
	int client_addr;
	int conn_fd;
} connection_args;

void *connection_handler(void *);

void ipaddrtstr(int ip, char *result)
{
	sprintf(result, "%d.%d.%d.%d", (ip) & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF,
			(ip >> 24) & 0xFF);

} // source: stackoverflow

sem_t conn_sem;

int main(int argc, char **argv)
{

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

	printf("Started server on port %d!\n", SERVER_PORT);

	struct sockaddr_in client_addr = {0};
	socklen_t client_addrlen = sizeof(client_addr);

	while (1)
	{
		int conn_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addrlen);

		if (conn_fd == -1)
		{
			fprintf(stderr, "Socket accept error in thread!\n");
			sem_post(&conn_sem);

			return -1;
		}

		// Accept connection
		if (sem_trywait(&conn_sem) == 0)
		{
			pthread_t thread;

			connection_args *args = malloc(sizeof(connection_args));
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
	connection_args *conn_args = (connection_args *) args;

	char ipstr[16];
	ipaddrtstr(conn_args->client_addr, ipstr);

	printf("Connected to %s!\n", ipstr);

	uint8_t start_seq;

	ssize_t res = recv(conn_args->conn_fd, &start_seq, 1, 0);

	if (res == 0)
	{
		printf("Disconnected from %s!\n", ipstr);

		close(conn_args->conn_fd);
		sem_post(&conn_sem);
		return NULL;
	}
	else if (res == -1)
	{
		perror("Recv error");

		close(conn_args->conn_fd);
		sem_post(&conn_sem);
		return NULL;
	}
	else if (start_seq != STARTING_SEQ)
	{
		printf("Recived malformed packet from %s, closing connection!\n", ipstr);

		close(conn_args->conn_fd);
		sem_post(&conn_sem);
		return NULL;
	}

	user_t user = {0};

	while (1)
	{

		/*
		 * int byte_letti = 0;
			while (byte_letti < payload_length) {
		int n = recv(socket_fd, buffer + byte_letti, payload_length - byte_letti, 0);

		if (n > 0) {
			byte_letti += n; // Accumula i byte letti
		} else if (n == 0) {
			// IL CLIENT HA CHIUSO LA CONNESSIONE PREMATURAMENTE!
			break;
		} else {
			// Errore della socket
			break;
		}
			}
		 */
		packet_header_t header;
		ssize_t res = recv(conn_args->conn_fd, &header, sizeof(packet_header_t), 0);

		header.payload_size = ntohs(header.payload_size);

		if (res == 0)
		{
			printf("Disconnected from %s!\n", ipstr);

			close(conn_args->conn_fd);
			sem_post(&conn_sem);
			return NULL;
		}
		else if (res == -1)
		{
			perror("Recv error");

			close(conn_args->conn_fd);
			sem_post(&conn_sem);
			return NULL;
		}

		switch (header.opcode)
		{
		case LOGIN:
			handle_login(&header, &user, conn_args->conn_fd);
			break;
		case REGISTER:
			break;
		case SEARCH:
			break;
		case ADD_CONTACT:
			break;
		default:
			// todo respond with proper status code instead of closing connection
			printf("Recived malformed packet! Closing connection with %s...\n", ipstr);
			close(conn_args->conn_fd);
			sem_post(&conn_sem);
			return NULL;
		}
	}

	return NULL;
}
