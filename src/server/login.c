#include "login.h"
#include "../common/utils.h"
#include "../protocol.h"
#include "database/user_db.h"
#include "sender.h"

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_username_password(const packet_header_t *header, char **username, char **password,
						   unsigned int *username_length, int conn_fd)
{
	if (header->payload_size < 32 + MIN_USERNAME_LEN ||
		header->payload_size - 32 > MAX_USERNAME_LEN)
	{
		if (!send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
		}
		return -1;
	}

	unsigned char buffer[header->payload_size];

	ssize_t read_bytes = recv_exact(conn_fd, buffer, header->payload_size, 0);

	if (read_bytes < 0)
	{
		perror("Error during receive!");
		return -1;
	}

	if (read_bytes < header->payload_size)
	{
		if (!send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
		}
		return -1;
	}

	*username_length = header->payload_size - 32;

	*username = malloc(sizeof(char) * (*username_length + 1));
	// password is a sha256 digest, not a printable string so no terminator required
	*password = malloc(sizeof(char) * 32);

	if (*username == NULL || *password == NULL)
	{
		if (!send_packet(conn_fd, SERVER_ERROR, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
		}
		perror("Error in malloc!");
		exit(-1);
	}

	memcpy(*username, buffer, header->payload_size - 32);
	memcpy(*password, buffer + header->payload_size - 32, 32);

	(*username)[*username_length] = '\0';

	return 1;
}

int handle_login(const packet_header_t *header, user_t *user, int conn_fd)
{
	char *username, *password;
	unsigned int username_length;

	if (read_username_password(header, &username, &password, &username_length, conn_fd) == -1)
	{
		return -1;
	}

	printf("Checking db against '%s'\n", username);

	if (!is_printable(username, username_length))
	{

		if (!send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
		}
		return -1;
	}

	auth_level_t auth_level;

	switch (check_credentials(username, password, &auth_level))
	{
	case 0:
		// invalid credentials
		if (!send_packet(conn_fd, INVALID_CREDENTIALS, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		return 0;
	case 1:
		free(user->username);
		free(user->password);

		user->username = username;
		user->password = password;
		user->auth_level = auth_level;

		if (!send_packet(conn_fd, OK, (unsigned char *) &auth_level, 1))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		break;
	case -1: // error
		if (!send_packet(conn_fd, SERVER_ERROR, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
		}
		return -1;
	}

	return 0;
}
