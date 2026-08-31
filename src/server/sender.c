#include "../protocol.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool send_packet(int conn_fd, status_t status, unsigned char *payload, uint16_t payload_size)
{
	if (payload_size > 0 && payload == NULL)
	{
		return false;
	}

	// Status: 1B + Payload size: 2B + Payload: ?B
	size_t buffer_size = 1 + 2 + payload_size;
	char buffer[buffer_size];

	buffer[0] = (char) status;

	uint16_t net_size = htons(payload_size);
	memcpy(&buffer[1], &net_size, sizeof(net_size));

	if (payload_size > 0)
	{
		memcpy(&buffer[3], payload, payload_size);
	}

	size_t bytes_sent = 0;

	while (bytes_sent < buffer_size)
	{
		ssize_t just_sent =
			send(conn_fd, buffer + bytes_sent, buffer_size - bytes_sent, MSG_NOSIGNAL);

		if (just_sent < 0)
		{
			// try again
			if (errno == EINTR)
			{
				continue;
			}
			// socket error or disconnected
			return false;
		}

		if (just_sent == 0)
		{
			// disconnected
			return false;
		}

		bytes_sent += (size_t) just_sent;
	}

	char *str;

	switch (status)
	{
	case OK:
		str = "OK";
		break;
	case SERVER_ERROR:
		str = "SERVER_ERROR";
		break;
	case MALFORMED_REQUEST:
		str = "MALFORMED_REQUEST";
		break;
	case NOT_FOUND:
		str = "NOT_FOUND";
		break;
	case UNAUTHORIZED:
		str = "UNAUTHORIZED";
		break;
	case INVALID_CREDENTIALS:
		str = "INVALID_CREDENTIALS";
		break;
	case USERNAME_TAKEN:
		str = "USERNAME_TAKEN";
		break;
	case ALL_RETURNED:
		str = "ALL_RETURNED";
		break;
	case FEWER_RETURNED:
		str = "FEWER_RETURNED";
		break;
	case CONTACT_ALREADY_EXISTS:
		str = "CONTACT_ALREADY_EXISTS";
		break;
	}

	printf("Sent response status: %s with payload size %d\n", str, payload_size);

	return true;
}

ssize_t discard_bytes(int fd, size_t count)
{
	size_t bytes_left = count;

	while (bytes_left > 0)
	{
		// NULL buffer + MSG_TRUNC scarta i byte direttamente nel kernel
		ssize_t res = recv(fd, NULL, bytes_left, MSG_TRUNC);

		if (res < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (res == 0)
		{
			// Peer ha chiuso la connessione (EOF)
			break;
		}

		bytes_left -= (size_t) res;
	}

	return (ssize_t) (count - bytes_left);
}
