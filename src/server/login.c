#include "login.h"

#include <stdio.h>
#include <stdlib.h>

void handle_login()
{
	char *mess_buf = calloc(max_read_size + 1, sizeof(char));

	if (mess_buf == NULL)
	{
		fprintf(stderr, "Buffer allocation error in thread!\n");
		close(conn_args->conn_fd);
		sem_post(&conn_sem);

		return NULL;
	}

	ssize_t bytes_received;

	while ((bytes_received = recv(conn_args->conn_fd, mess_buf, max_read_size, 0)) > 0)
	{
		// Add terminator incase client has not
		mess_buf[bytes_received] = '\0';

		printf("Recived: %s\n", mess_buf);

		memset(mess_buf, 0, max_read_size + 1);
	}

	if (bytes_received == 0)
	{
		printf("Disconnected from %s!\n", ipstr);
	}
	else
	{
		perror("Recv error");
	}

	free(mess_buf);
	close(conn_args->conn_fd);
	sem_post(&conn_sem);
}
