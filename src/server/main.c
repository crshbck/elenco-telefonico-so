#include "../common/utils.h"
#include "../config.h"
#include "../protocol.h"
#include "database/contact_db.h"
#include "database/user_db.h"
#include "delete.h"
#include "insert.h"
#include "login.h"
#include "register.h"
#include "search.h"
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

typedef struct _connection_args
{
	int client_addr;
	int conn_fd;
} connection_args;

void *connection_handler(void *);
void *input_handler(void *);

void ipaddrtstr(int ip, char *result)
{
	sprintf(result, "%d.%d.%d.%d", (ip) & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF,
			(ip >> 24) & 0xFF);

} // source: stackoverflow

sem_t conn_sem;

int main(int argc, char **argv)
{
	int port = argc > 1 ? atoi(argv[1]) : SERVER_DEFAULT_PORT;

	if (!init_user_db())
	{
		error("User db initialization errror!");
	}

	if (!init_contact_db())
	{
		error("Contact db initialization error!");
	}

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
	sock_addr.sin_port = htons(port);
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

	printf("Started server on port %d!\n"
		   "Commands:\n"
		   "`perm` - Change user permissions\n\n",
		   port);

	struct sockaddr_in client_addr = {0};
	socklen_t client_addrlen = sizeof(client_addr);

	pthread_t input_thread;
	if (pthread_create(&input_thread, NULL, input_handler, NULL) != 0)
	{
		perror("Error starting input listening thread!");
		exit(-1);
	}

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

			if (pthread_create(&thread, NULL, connection_handler, (void *) args) != 0)
			{
				perror("Error starting connection thread!");
				exit(-1);
			}
		}
		// Refuse connection
		else
		{
			printf("Cannot accept connection!\n");
			send_packet(conn_fd, SERVER_ERROR, NULL, 0);
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

	user_t user = {0};

	while (1)
	{
		// if (user.username == NULL)
		// {
		// 	printf("User is not logged in\n");
		// }
		// else
		// {
		// 	switch (user.auth_level)
		// 	{
		// 	case USER:
		// 		printf("User is logged in as [USER]");
		// 		break;
		// 	case ADMIN:
		// 		printf("User is logged in as [ADMIN]");
		// 		break;
		// 	default:
		// 		break;
		// 	}
		// 	printf(" '%s'\n", user.username);
		// }

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
			perror("Recv error starting seq");

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

		unsigned char _header[3];

		res = recv(conn_args->conn_fd, &_header, 3, 0);

		if (res == 0)
		{
			printf("Disconnected from %s!\n", ipstr);

			close(conn_args->conn_fd);
			sem_post(&conn_sem);
			return NULL;
		}
		else if (res == -1)
		{
			perror("Recv error header");

			close(conn_args->conn_fd);
			sem_post(&conn_sem);
			return NULL;
		}
		else if (res < 3)
		{
			send_packet(conn_args->conn_fd, MALFORMED_REQUEST, NULL, 0);

			close(conn_args->conn_fd);
			sem_post(&conn_sem);
			return NULL;
		}

		packet_header_t header;

		header.opcode = _header[0];
		header.payload_size = (_header[1] << 8) + _header[2];

#ifdef DEBUG
		printf("Opcode: 0x%02X\n", header.opcode);
		printf("Payload_size: %d\n", header.payload_size);
#endif

		switch (header.opcode)
		{
		case LOGIN:
			if (handle_login(&header, &user, conn_args->conn_fd) == -1)
			{
				close(conn_args->conn_fd);
				sem_post(&conn_sem);
				return NULL;
			}
			break;
		case REGISTER:
			if (handle_register(&header, &user, conn_args->conn_fd) == -1)
			{
				close(conn_args->conn_fd);
				sem_post(&conn_sem);
				return NULL;
			}
			break;
		case INSERT:
			if (user.auth_level != ADMIN)
			{
				send_packet(conn_args->conn_fd, UNAUTHORIZED, NULL, 0);
				discard_bytes(conn_args->conn_fd, header.payload_size);
			}
			else
			{
				if (handle_insert(&header, conn_args->conn_fd) == -1)
				{
					close(conn_args->conn_fd);
					sem_post(&conn_sem);
					return NULL;
				}
			}
			break;
		case DELETE:
			if (user.auth_level != ADMIN)
			{
				send_packet(conn_args->conn_fd, UNAUTHORIZED, NULL, 0);
				discard_bytes(conn_args->conn_fd, header.payload_size);
			}
			else
			{
				if (handle_delete(&header, conn_args->conn_fd) == -1)
				{
					close(conn_args->conn_fd);
					sem_post(&conn_sem);
					return NULL;
				}
			}
			break;
		case SEARCH:
			if (user.auth_level != ADMIN && user.auth_level != USER)
			{
				send_packet(conn_args->conn_fd, UNAUTHORIZED, NULL, 0);
				discard_bytes(conn_args->conn_fd, header.payload_size);
			}
			else
			{
				if (handle_search(&header, conn_args->conn_fd) == -1)
				{
					close(conn_args->conn_fd);
					sem_post(&conn_sem);
					return NULL;
				}
			}
			break;
		default:
			printf("Recived malformed packet! Closing connection with %s...\n", ipstr);
			send_packet(conn_args->conn_fd, MALFORMED_REQUEST, NULL, 0);
			close(conn_args->conn_fd);
			sem_post(&conn_sem);
			return NULL;
		}
	}

	return NULL;
}

void *input_handler(void *args)
{
	char input_buffer[32];
	while (1)
	{
		if (fgets(input_buffer, 32, stdin) == NULL)
		{
			return (void *) -1;
		}

		input_buffer[strcspn(input_buffer, "\r\n")] = '\0';

		if (strcmp(input_buffer, "quit") == 0)
		{
			return 0;
		}
		else if (strcmp(input_buffer, "perm") == 0)
		{
			printf("Insert username: ");

			char username_buffer[MAX_USERNAME_LEN];

			if (fgets(username_buffer, MAX_USERNAME_LEN, stdin) == NULL)
			{
				return (void *) -1;
			}

			username_buffer[strcspn(username_buffer, "\r\n")] = '\0';

			printf("Auth level:\n"
				   "0: Anonymous\n"
				   "1: User\n"
				   "2: Admin\n"
				   "q: Quit\n\n");
			if (fgets(input_buffer, 32, stdin) == NULL)
			{
				return (void *) -1;
			}

			auth_level_t auth_level;

			if (input_buffer[0] == 'q')
			{
				continue;
			}

			if (input_buffer[0] < '0' || input_buffer[0] > '2')
			{
				printf("Invalid selection!");
				continue;
			}

			auth_level = input_buffer[0] - '0';

			switch (change_permissions(username_buffer, auth_level))
			{
			case 0:
				printf("User not found!\n");
				continue;
				break;
			case 1:
				printf("Success!\n");
				continue;
				break;
			case -1:
				printf("Error!\n");
				continue;
				break;
			}
		}
		else
		{
			printf("Unknown operation '%s'!\n", input_buffer);
			continue;
		}
	}
}
