#include "utils.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

void error(const char *m)
{
	perror(m);
	exit(-1);
}

ssize_t recv_exact(int fd, void *buf, size_t count, int flags)
{
	size_t bytes_left = count;
	char *ptr = (char *) buf;

	while (bytes_left > 0)
	{
		ssize_t bytes_read = recv(fd, ptr, bytes_left, flags);

		if (bytes_read < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			// Errore di I/O o socket
			return -1;
		}
		else if (bytes_read == 0)
		{
			// Connessione chiusa dal peer (EOF)
			break;
		}

		bytes_left -= (size_t) bytes_read;
		ptr += bytes_read;
	}

	return (ssize_t) (count - bytes_left);
}

ssize_t send_exact(int fd, const void *buf, size_t count, int flags)
{
	size_t bytes_left = count;
	const char *ptr = (const char *) buf;

	while (bytes_left > 0)
	{
		ssize_t bytes_sent = send(fd, ptr, bytes_left, flags);

		if (bytes_sent < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			// Errore di socket/connessione (es. EPIPE, ECONNRESET)
			return -1;
		}
		else if (bytes_sent == 0)
		{
			// Nessun byte inviato (interruzione anomala)
			break;
		}

		bytes_left -= (size_t) bytes_sent;
		ptr += bytes_sent;
	}

	return (ssize_t) (count - bytes_left);
}

bool is_printable(const char *string, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (string[i] < 0x20 || string[i] > 0x7E)
		{
			return false;
		}
	}

	return true;
}
