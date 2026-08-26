#include "register.h"
#include "common.h"
#include "database/user_db.h"
#include "login.h"
#include "sender.h"
#include <stdio.h>
#include <stdlib.h>

int handle_register(const packet_header_t *header, user_t *user, int conn_fd)
{
	if (header->payload_size > 32 + MAX_USERNAME_LEN)
	{
		send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0);
		return 0;
	}

	char *username, *password;
	int username_length;

	if (read_username_password(header, &username, &password, &username_length, conn_fd) == -1)
	{
		return -1;
	}

	if (username_length > MAX_USERNAME_LEN)
	{
		// todo check return status
		send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0);
		return 0;
	}

	if (!check_username(username, username_length))
	{
		// todo check return status
		send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0);
		return 0;
	}

	auth_level_t auth_level;

	switch (check_credentials(username, password, &auth_level))
	{
	case 0:
		// user not already registered
		break;
	case 1:
		// user already registered
		// todo check return status
		send_packet(conn_fd, INVALID_CREDENTIALS, NULL, 0);
		return 0;
		break;
	case -1: // error
		// todo check return status
		send_packet(conn_fd, SERVER_ERROR, NULL, 0);
		return 0;
		break;
	}

#ifdef DEBUG
	printf("Registering user %s\n", username);
#endif

	free(user->username);
	free(user->password);

	user->username = username;
	user->password = password;

	switch (add_user(user))
	{
	case 0:
		// ok
		// todo check return status
		send_packet(conn_fd, OK, NULL, 0);
		break;
	case -1:
		send_packet(conn_fd, SERVER_ERROR, NULL, 0);
		break;
	}

	return 1;
}
