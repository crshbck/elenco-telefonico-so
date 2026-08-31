
#include "contact_db.h"
#include "../../config.h"
#include "db_utils.h"
#include "sync.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * _________________________________________________________________________________________________________
 * | ACTIVE (1B) | NAME_LENGTH (1B) | PHONE_NUMBER_LENGTH (1B) | NAME (variable) | PHONE_NUMBER
 * (variable) |
 * _________________________________________________________________________________________________________
 */

static int contact_db_fd = -1;

static int semaphore;

bool init_contact_db()
{
	if (!rwlock_init(&semaphore))
	{
		return false;
	}

	contact_db_fd = open(CONTACT_DB_FILENAME, O_RDWR | O_CREAT, 0640);

	if (contact_db_fd == -1)
	{
		rwlock_destroy(&semaphore);
		return false;
	}

	return true;
}

bool close_contact_db()
{
	if (!rwlock_get_writer(&semaphore))
	{
		return false;
	}

	if (close(contact_db_fd) == -1)
	{
		rwlock_destroy(&semaphore);
		return false;
	}

	contact_db_fd = -1;

	if (rwlock_destroy(&semaphore) == -1)
	{
		return false;
	}

	return true;
}

static int _unlocked_add_contact(const char *name, const char *phone_number, uint8_t name_length,
								 uint8_t phone_number_length)
{
	off_t offset = 0;

	unsigned char header[3];

	// first search for inactive contact with same record length, if not found append user to file
	while (1)
	{
		ssize_t bytes_read = pread_exact(contact_db_fd, header, 3, offset);

		// EOF
		if (bytes_read == 0)
		{
			break;
		}

		// error
		if (bytes_read == -1)
		{
			return -1;
		}

		// corrupted database
		if (bytes_read < 3)
		{
			return -2;
		}

		offset += 3;

		// check if inactive and size fits, if it does, overwrite
		if (header[0] == 0 && (header[1] + header[2]) == (name_length + phone_number_length))
		{
			offset -= 3;
			break;
		}
		else
		{
			// skip record
			offset += header[1] + header[2];
		}
	}

	int record_size = 3 + name_length + phone_number_length;

	char contact_buf[record_size];

	// ACTIVE = 1
	contact_buf[0] = 1;

	// NAME_LENGTH
	contact_buf[1] = name_length;

	// PHONE_NUMBER_LENGTH
	contact_buf[2] = phone_number_length;

	// NAME
	memcpy(&contact_buf[3], name, name_length);

	// PHONE_NUMBER
	memcpy(&contact_buf[3] + name_length, phone_number, phone_number_length);

	ssize_t bytes_written = pwrite_exact(contact_db_fd, contact_buf, record_size, offset);

	if (bytes_written < record_size)
	{
		return -1;
	}

	return 1;
}

static int _unlocked_delete_contact(const char *name, uint8_t name_length)
{
	// variable length array used instead of heap to prevent memory leak across multiple early
	// returns, name_length is limited anyways so stack overflow should not be an issue here,
	// validation against MAX_NAME_LENGTH is mandatory before this call
	char buffer[name_length];

	off_t offset = 0;

	while (1)
	{
		unsigned char header[3];

		ssize_t bytes_read = pread_exact(contact_db_fd, header, 3, offset);

		offset += 3;

		if (bytes_read == 0)
		{
			// EOF
			return 0;
		}

		if (bytes_read == -1)
		{
			// i/o error
			return -1;
		}

		if (bytes_read < 3)
		{
			// corrupted database
			return -2;
		}

		if (header[0] == 1 && header[1] == name_length)
		{
			bytes_read = pread_exact(contact_db_fd, buffer, name_length, offset);
			offset += name_length;

			if (bytes_read == -1)
			{
				return -1;
			}

			if (bytes_read < name_length)
			{
				// corrupted database
				return -2;
			}

			if (memcmp(buffer, name, name_length) == 0)
			{
				// contact to be deleted found
				// head back name_length + header bytes and deactivate user
				offset -= 3 + name_length;

				char zero = 0;

				if (pwrite_exact(contact_db_fd, &zero, 1, offset) < 1)
				{
					return -1;
				}

				return 1;
			}
			else
			{
				offset += header[2];
			}
		}
		else
		{
			// contact had either same name length but not same name, skip record or was inactive
			offset += header[1] + header[2];
		}
	}
}

int memcmpcaseins(const char *s1, const char *s2, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		char c1 = tolower(s1[i]);
		char c2 = tolower(s2[i]);

		if (c1 != c2)
		{
			return c1 - c2;
		}
	}
	return 0;
}

static int _unlocked_search_contact(const char *query, size_t query_length, uint8_t limit,
									char ***buf, size_t *match_count, size_t *buf_size)
{
	off_t offset = 0;

	*buf = malloc(sizeof(char *) * limit);
	*match_count = 0;
	*buf_size = 0;

	while (1)
	{
		char header[3];
		ssize_t bytes_read = pread_exact(contact_db_fd, header, 3, offset);

		offset += 3;

		if (bytes_read == 0)
		{
			// EOF
			return 0;
		}

		if (bytes_read == -1)
		{
			return -1;
		}

		if (bytes_read < 3)
		{
			// corrupted database
			return -2;
		}

		unsigned char is_active = header[0];
		unsigned char name_length = header[1];
		unsigned char phone_number_length = header[2];

		if (is_active == 1 && name_length >= query_length)
		{
			char *record = malloc(sizeof(char) * (2 + name_length + phone_number_length));

			record[0] = name_length;
			record[1] = phone_number_length;

			bytes_read = pread_exact(contact_db_fd, record + 2, name_length, offset);

			if (bytes_read == -1)
			{
				// i/o error
				return -1;
			}

			if (bytes_read < name_length)
			{
				// corrupted database
				return -2;
			}

			offset += name_length;

			if (memcmpcaseins(record + 2, query, query_length) == 0)
			{
				// add phone number
				bytes_read = pread_exact(contact_db_fd, record + 2 + name_length,
										 phone_number_length, offset);

				if (bytes_read == -1)
				{
					// i/o error
					free(record);
					return -1;
				}

				if (bytes_read < phone_number_length)
				{
					// corrupted database
					free(record);
					return -2;
				}

				(*buf)[*match_count] = record;

				*buf_size += 2 + name_length + phone_number_length;
				++(*match_count);

				if (*match_count == limit)
				{
					return 0;
				}
			}
			else
			{
				free(record);
			}

			offset += phone_number_length;
		}
		else
		{
			// just skip the rest of the record
			offset += name_length + phone_number_length;
		}
	}
}

void free_query_buffer(char **buf, size_t len)
{
	for (int i = 0; i < len; i++)
	{
		free(buf[i]);
	}
	free(buf);
}

int add_contact(const char *name, const char *phone_number, uint8_t name_length,
				uint8_t phone_number_length)
{
	if (!rwlock_get_writer(&semaphore))
	{
		return -1;
	}

	int status = _unlocked_add_contact(name, phone_number, name_length, phone_number_length);

	if (!rwlock_release_writer(&semaphore))
	{
		return -1;
	}

	return status;
}

int delete_contact(const char *name, uint8_t name_length)
{
	if (!rwlock_get_writer(&semaphore))
	{
		return -1;
	}

	int status = _unlocked_delete_contact(name, name_length);

	if (!rwlock_release_writer(&semaphore))
	{
		return -1;
	}

	return status;
}

int search_contact(const char *query, size_t query_length, uint8_t limit, char ***buf,
				   size_t *match_count, size_t *buf_size)
{
	if (!rwlock_get_reader(&semaphore))
	{
		return -1;
	}

	int status = _unlocked_search_contact(query, query_length, limit, buf, match_count, buf_size);

	if (!rwlock_release_reader(&semaphore))
	{
		return -1;
	}

	return status;
}
