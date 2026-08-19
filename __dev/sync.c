#include "sync.h"

#include <stdbool.h>

#include <fcntl.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool rwlock_init(int *semaphore)
{
	*semaphore = semget(IPC_PRIVATE, 3, 0660);

	if (*semaphore == -1)
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
	if (semctl(*semaphore, 0, SETALL, args) == -1)
	{
		return false;
	}

	return true;
}

bool rwlock_get_reader(int *semaphore)
{
	struct sembuf semops[2];
	// Wait for SEM_WRITER_WAITING == 0
	semops[0] = (struct sembuf) {.sem_num = 1, .sem_op = 0, .sem_flg = 0};
	// SEM_READERS += 1
	semops[1] = (struct sembuf) {.sem_num = 0, .sem_op = +1, .sem_flg = 0};

	if (semop(*semaphore, semops, 2) == -1)
	{
		return false;
	}

	return true;
}

bool rwlock_release_reader(int *semaphore)
{
	struct sembuf op;
	// SEM_READERS -= 1
	op = (struct sembuf) {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
	if (semop(*semaphore, &op, 1) == -1)
	{
		return false;
	}

	return true;
}

bool rwlock_get_writer(int *semaphore)
{
	struct sembuf semops[2];
	// SEM_WRITER_WAITING += 1
	semops[0] = (struct sembuf) {.sem_num = 1, .sem_op = +1, .sem_flg = 0};

	if (semop(*semaphore, semops, 1) == -1)
	{
		return false;
	}

	// MUTEX_WRITING -= 1
	semops[0] = (struct sembuf) {.sem_num = 2, .sem_op = -1, .sem_flg = 0};
	// Wait for SEM_READERS == 0
	semops[1] = (struct sembuf) {.sem_num = 0, .sem_op = 0, .sem_flg = 0};

	if (semop(*semaphore, semops, 2) == -1)
	{
		return false;
	}

	return true;
}

bool rwlock_release_writer(int *semaphore)
{
	struct sembuf semops[2];

	// SEM_WRITERS_WAITING -= 1
	semops[0] = (struct sembuf) {.sem_num = 1, .sem_op = -1, .sem_flg = 0};
	// MUTEX_WRITING += 1
	semops[1] = (struct sembuf) {.sem_num = 2, .sem_op = +1, .sem_flg = 0};

	if (semop(*semaphore, semops, 2) == -1)
	{
		return false;
	}

	return true;
}

int rwlock_destroy(int *semaphore)
{
	if (*semaphore != -1)
	{
		*semaphore = -1;
		return semctl(*semaphore, 0, IPC_RMID);
	}

	return -1;
}
