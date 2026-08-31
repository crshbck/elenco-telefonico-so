#include "register.h"
#include "../common/utils.h"
#include "database/user_db.h"
#include "login.h"
#include "sender.h"
#include <stdio.h>
#include <stdlib.h>

int handle_register(const packet_header_t *header, user_t *user, int conn_fd)
{
	char *username, *password;
	unsigned int username_length;

	if (read_username_password(header, &username, &password, &username_length, conn_fd) == -1)
	{
		return -1;
	}

	if (!is_printable(username, username_length))
	{
		if (!send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
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
		if (!send_packet(conn_fd, USERNAME_TAKEN, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		return 0;
		break;
	case -1:
		// error
		if (!send_packet(conn_fd, SERVER_ERROR, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		return 0;
		break;
	}

	printf("Registering user %s\n", username);

	switch (add_user(user))
	{
	case 0:
		// ok
		free(user->username);
		free(user->password);

		user->username = username;
		user->password = password;
		user->auth_level = 0;

		if (!send_packet(conn_fd, OK, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		break;
	case -1:
		if (!send_packet(conn_fd, SERVER_ERROR, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		break;
	}

	return 1;
}
