#include "user_db.h"

#include "../src/config.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * ____________________________________________________________________________________________
 * | ACTIVE (1B) | USERNAME_LEN (1B) | USERNAME (variable) | PASSWORD (32B) | AUTH_LEVEL (1B) |
 * ____________________________________________________________________________________________
 */

int user_db_fd;

int semaphore;

bool init_user_db()
{
	semaphore = semget(IPC_PRIVATE, 3, 0660);

	if (semaphore == -1)
	{
		return false;
	}

	// as per spec
	union semun
	{
		int val;			   /* Value for SETVAL */
		struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
		unsigned short *array; /* Array for GETALL, SETALL */
		struct seminfo *__buf; /* Buffer for IPC_INFO
								  (Linux-specific) */
	};

	union semun args;
	args.array = (unsigned short[]) {0, 0, 1};

	// 0: SEM_READERS
	// 1: SEM_WRITER_WAITING
	// 2: MUTEX_WRITING
	if (semctl(semaphore, 0, SETALL, args) == -1)
	{
		return false;
	}

	user_db_fd = open(USER_DB_FILENAME, O_RDWR | O_CREAT, 0640);

	if (user_db_fd == -1)
	{
		return false;
	}

	return true;
}

int _unlocked_check_credentials(const char *username, const char *password,
								auth_level_t *auth_level)
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

int check_credentials(const char *username, const char *password, auth_level_t *auth_level)
{

	struct sembuf semops[2];
	// Wait for SEM_WRITER_WAITING == 0
	semops[0] = (struct sembuf) {.sem_num = 1, .sem_op = 0, .sem_flg = 0};
	// SEM_READERS += 1
	semops[1] = (struct sembuf) {.sem_num = 0, .sem_op = +1, .sem_flg = 0};

	if (semop(semaphore, semops, 2) == -1)
	{
		return -1;
	}

	int status = _unlocked_check_credentials(username, password, auth_level);

	// SEM_READERS -= 1
	semops[0] = (struct sembuf) {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
	if (semop(semaphore, semops, 1) == -1)
	{
		return -1;
	}

	return status;
}

bool _unlocked_add_user(const user_t *user)
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

bool add_user(const user_t *user)
{
	struct sembuf semops[2];
	// SEM_WRITER_WAITING += 1
	semops[0] = (struct sembuf) {.sem_num = 1, .sem_op = +1, .sem_flg = 0};

	if (semop(semaphore, semops, 1) == -1)
	{
		return -1;
	}

	// MUTEX_WRITING -= 1
	semops[0] = (struct sembuf) {.sem_num = 2, .sem_op = -1, .sem_flg = 0};
	// Wait for SEM_READERS == 0
	semops[1] = (struct sembuf) {.sem_num = 0, .sem_op = 0, .sem_flg = 0};

	if (semop(semaphore, semops, 1) == -1)
	{
		return -1;
	}

	bool status = _unlocked_add_user(user);

	// SEM_WRITERS_WAITING -= 1
	semops[0] = (struct sembuf) {.sem_num = 1, .sem_op = -1, .sem_flg = 0};
	// MUTEX_WRITING += 1
	semops[1] = (struct sembuf) {.sem_num = 2, .sem_op = +1, .sem_flg = 0};

	if (semop(semaphore, semops, 2) == -1)
	{
		return -1;
	}

	return status;
}
