#include "login.h"
#include "../protocol.h"
#include "database/query.h"
#include "sender.h"

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool check_username(const char *username, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (username[i] < 0x20 || username[i] > 0x7E)
		{
			return false;
		}
	}
	printf("\n");

	return true;
}

void handle_login(const packet_header_t *header, user_t *user, int conn_fd)
{
	char buffer[header->payload_size];

	size_t read_bytes = 0;

	while (read_bytes < header->payload_size)
	{
		ssize_t res =
			recv(conn_fd, (char *) buffer + read_bytes, header->payload_size - read_bytes, 0);

		if (res > 0)
		{
			read_bytes += res;
		}
		else if (res == 0)
		{
			// EOF
			break;
		}
		else
		{
			perror("Error during recive!");
			break;
		}
	}
	if (read_bytes < header->payload_size)
	{
		// todo check return status
		send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0);
		return;
	}

	if (header->payload_size < MAX_PASSWORD_LEN + MIN_USERNAME_LEN &&
		header->payload_size - MAX_PASSWORD_LEN > MAX_USERNAME_LEN)
	{
		// todo check return status
		send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0);
		return;
	}

	int username_length = header->payload_size - 32;

	char *username = malloc(sizeof(char) * (username_length + 1));
	char *password = malloc(sizeof(char) * (MAX_PASSWORD_LEN + 1));

	if (username == NULL || password == NULL)
	{
		// todo check return status
		send_packet(conn_fd, SERVER_ERROR, NULL, 0);
		perror("Error in malloc!");
		exit(-1);
	}

	memcpy(username, buffer, header->payload_size - MAX_PASSWORD_LEN);
	memcpy(password, buffer + header->payload_size - MAX_PASSWORD_LEN, MAX_PASSWORD_LEN);

	username[username_length] = '\0';
	password[MAX_PASSWORD_LEN] = '\0';

	if (!check_username(username, username_length))
	{
		// todo check return status
		send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0);
		return;
	}

	user_t *queried_user = query_user(username, password);

	printf("User '%s' with auth level '%d' has logged in!\n", queried_user->username,
		   queried_user->auth_level);

	if (queried_user == NULL)
	{
		// todo check return status
		send_packet(conn_fd, WRONG_CREDENTIALS, NULL, 0);
		return;
	}

	user = queried_user;

	char auth_level = queried_user->auth_level;

	// todo check return status
	send_packet(conn_fd, OK, &auth_level, 1);
}
