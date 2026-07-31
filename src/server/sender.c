#include "../protocol.h"

#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

bool send_packet(int conn_fd, status_t status, char *payload, uint16_t payload_size)
{
	// Status: 1B + Payload size: 2B + Payload: ?B
	size_t buffer_size = 1 + 2 + payload_size;

	char buffer[buffer_size];

	buffer[0] = (char) status;

	uint16_t net_size = htons(payload_size);
	memcpy(&buffer[1], &net_size, sizeof(net_size));

	if (payload_size > 0 && payload != NULL)
	{
		memcpy(&buffer[3], payload, payload_size);
	}

	size_t bytes_sent = 0;

	while (bytes_sent < buffer_size)
	{
		ssize_t just_sent =
			send(conn_fd, buffer + bytes_sent, buffer_size - bytes_sent, MSG_NOSIGNAL);

		// Disconnected
		if (bytes_sent <= 0)
		{
			return false;
		}

		bytes_sent += (size_t) just_sent;
	}

	return true;
}
