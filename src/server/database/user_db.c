#include "user_db.h"
#include "sync.h"

#include "../../config.h"
#include "db_utils.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * ______________________________________________________________________________
 * | USERNAME_LEN (1B) | AUTH_LEVEL (1B) | PASSWORD (32B) | USERNAME (variable) |
 * ______________________________________________________________________________
 */

static int user_db_fd = -1;

static int semaphore;

bool init_user_db()
{
	if (!rwlock_init(&semaphore))
	{
		return false;
	}

	user_db_fd = open(USER_DB_FILENAME, O_RDWR | O_CREAT, 0640);

	if (user_db_fd == -1)
	{
		rwlock_destroy(&semaphore);

		return false;
	}

	return true;
}

bool close_user_db()
{

	if (close(user_db_fd) == -1)
	{
		rwlock_destroy(&semaphore);
		return false;
	}

	user_db_fd = -1;

	if (rwlock_destroy(&semaphore) == -1)
	{
		return false;
	}

	return true;
}

static int _unlocked_check_credentials(const char *username, const char *password,
									   auth_level_t *auth_level)
{
	off_t offset = 0;

	unsigned char header;
	ssize_t bytes_read;

	while (1)
	{
		bytes_read = pread_exact(user_db_fd, &header, 1, offset);

		if (bytes_read == 0)
		{
			// EOF
			break;
		}

		offset += 1;

		if (bytes_read == -1)
		{
			// error
			return -1;
		}

		int record_size = 1 + 32 + header;

		if (header != strlen(username))
		{
			// skip auth level + password + username
			offset += record_size;
			continue;
		}

		// auth level + password (fixed) + username
		unsigned char buffer[record_size];

		bytes_read = pread_exact(user_db_fd, buffer, record_size, offset);

		if (bytes_read == 0)
		{
			// EOF
			break;
		}

		offset += record_size;

		if (bytes_read == -1)
		{
			// error
			return -1;
		}

		if (memcmp(&buffer[1 + 32], username, strlen(username)) == 0 &&
			memcmp(&buffer[1], password, 32) == 0)
		{
			*auth_level = buffer[0];

			return 1;
		}
	}

	return 0;
}

int check_credentials(const char *username, const char *password, auth_level_t *auth_level)
{

	if (!rwlock_get_reader(&semaphore))
	{
		return -1;
	}

	int status = _unlocked_check_credentials(username, password, auth_level);

	if (!rwlock_release_reader(&semaphore))
	{
		return -1;
	}

	return status;
}

static int _unlocked_add_user(const user_t *user)
{
	int record_size = 1 + strlen(user->username) + 32 + 1;

	unsigned char buffer[record_size];

	// Username length
	buffer[0] = strlen(user->username);

	// Auth level
	buffer[1] = user->auth_level;

	// Password
	memcpy(&buffer[2], user->password, 32);

	// Username
	memcpy(&buffer[1 + 1 + 32], user->username, strlen(user->username));

	ssize_t bytes_written = 0;

	off_t offset = lseek(user_db_fd, 0, SEEK_END);

	if (offset == -1)
	{
		return -1;
	}

	bytes_written = pwrite_exact(user_db_fd, buffer + bytes_written, record_size, offset);

	if (bytes_written < record_size)
	{
		return -1;
	}

	return 0;
}

int add_user(const user_t *user)
{
	if (!rwlock_get_writer(&semaphore))
	{
		return -1;
	}

	int status = _unlocked_add_user(user);

	if (!rwlock_release_writer(&semaphore))
	{
		return -1;
	}

	return status;
}
