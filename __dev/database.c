#include "database.h"

#include "../src/config.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * ____________________________________________________________________________________________
 * | ACTIVE (1B) | USERNAME_LEN (1B) | USERNAME (variable) | PASSWORD (32B) | AUTH_LEVEL (1B) |
 * ____________________________________________________________________________________________
 *
 */

int user_db_fd;
int contact_db_fd;

bool init_db()
{
	user_db_fd = open(USER_DB_FILENAME, O_RDWR | O_CREAT, 0640);
	contact_db_fd = open(CONTACT_DB_FILENAME, O_RDWR | O_CREAT, 0640);

	if (user_db_fd == -1 || contact_db_fd == -1)
	{
		return false;
	}

	return true;
}

int check_credentials(const char *username, const char *password, auth_level_t *auth_level)
{
	if (lseek(user_db_fd, 0, SEEK_SET) == -1)
	{
		return -1;
	}

	unsigned char header[2];
	ssize_t bytes_read;

	while (1)
	{
		// TODO: iterate read
		bytes_read = read(user_db_fd, header, 2);

		if (bytes_read == 0)
		{
			// EOF
			break;
		}

		if (bytes_read == -1)
		{
			// error
			return -1;
		}

		if (bytes_read < 2)
		{
			// corrupted file
			return -2;
		}

		if (header[0] == 0 || header[1] != strlen(username))
		{
			// skip username + password + auth level
			if (lseek(user_db_fd, header[1] + 32 + 1, SEEK_CUR) == -1)
			{
				return -1;
			}
			continue;
		}

		// username + password (fixed) + auth level
		unsigned char *buffer = malloc(sizeof(unsigned char) * (header[1] + 32 + 1));
		// TODO: iterate read
		bytes_read = read(user_db_fd, buffer, sizeof(buffer));

		if (bytes_read == 0)
		{
			// EOF
			break;
		}
		if (bytes_read == -1)
		{
			// error
			return -1;
		}

		if (memcmp(&buffer[0], username, strlen(username)) == 0 &&
			memcmp(&buffer[header[1]], password, 32) == 0)
		{
			*auth_level = buffer[sizeof(buffer) - 1];

			return 1;
		}
	}

	return 0;
}

bool add_user(const user_t *user)
{
	if (lseek(user_db_fd, 0, SEEK_END) == -1)
	{
		return false;
	}

	int record_size = 1 + 1 + strlen(user->username) + 32 + 1;

	unsigned char *buffer = malloc(sizeof(char) * record_size);

	// Active
	buffer[0] = 1;

	// Username length
	buffer[1] = strlen(user->username);

	// Username
	memcpy(&buffer[2], user->username, strlen(user->username));

	// Password
	memcpy(&buffer[strlen(user->username) + 2], user->password, 32);

	// Auth level
	buffer[record_size - 1] = user->auth_level;

	ssize_t bytes_written = 0, just_written = 0;

	while (bytes_written < record_size)
	{
		just_written = write(user_db_fd, buffer + bytes_written, record_size - bytes_written);

		if (just_written <= 0)
		{
			// Error
			return false;
		}

		bytes_written += just_written;
	}

	return true;
}
