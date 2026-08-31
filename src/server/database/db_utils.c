#include "db_utils.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

ssize_t pread_exact(int fd, void *buf, size_t count, off_t offset)
{
	size_t bytes_left = count;
	char *ptr = (char *) buf;
	off_t cur_offset = offset;

	while (bytes_left > 0)
	{
		ssize_t bytes_read = pread(fd, ptr, bytes_left, cur_offset);

		if (bytes_read < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			// i/o error
			return -1;
		}
		else if (bytes_read == 0)
		{
			// EOF
			break;
		}

		bytes_left -= (size_t) bytes_read;
		ptr += bytes_read;
		cur_offset += bytes_read;
	}

	return (ssize_t) (count - bytes_left);
}

ssize_t pwrite_exact(int fd, const void *buf, size_t count, off_t offset)
{
	size_t bytes_left = count;
	const char *ptr = (const char *) buf;
	off_t cur_offset = offset;

	while (bytes_left > 0)
	{
		ssize_t bytes_written = pwrite(fd, ptr, bytes_left, cur_offset);

		if (bytes_written <= 0)
		{
			if (bytes_written < 0 && errno == EINTR)
			{
				continue;
			}
			// i/o error
			return -1;
		}

		bytes_left -= (size_t) bytes_written;
		ptr += bytes_written;
		cur_offset += bytes_written;
	}

	return (ssize_t) count;
}
