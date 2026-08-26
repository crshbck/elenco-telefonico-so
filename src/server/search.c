#include "search.h"

#include "../common/utils.h"
#include "database/contact_db.h"
#include "sender.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int handle_search(const packet_header_t *header, int conn_fd)
{
	char buffer[header->payload_size];

	ssize_t read_bytes = recv_exact(conn_fd, buffer, header->payload_size, 0);

	if (read_bytes == -1)
	{
		perror("Error during receive!");
		return -1;
	}

	if (read_bytes < header->payload_size)
	{
		// todo check return status
		send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0);
		return 0;
	}

	int limit = buffer[0];
	char *name = &buffer[1];

	char **_raw_search_buf;
	size_t search_buf_size;
	size_t match_count;

	switch (search_contact(name, header->payload_size - 1, limit, &_raw_search_buf, &match_count,
						   &search_buf_size))
	{
	case 1:
		break;
	case -1:
		send_packet(conn_fd, SERVER_ERROR, NULL, 0);
		perror("Database I/O error! Terminating...");
		exit(-1);
		break;
	case -2:
		send_packet(conn_fd, SERVER_ERROR, NULL, 0);
		fprintf(stderr, "Database is corrupted! Terminating...\n");
		exit(-1);
		break;
	}

	status_t status;

	if (match_count < limit)
	{
		status = ALL_RETURNED;
	}
	else
	{
		status = FEWER_RETURNED;
	}

	char *search_buf = malloc(sizeof(char) * search_buf_size);

	size_t offset = 0;

	for (int i = 0; i < match_count; i++)
	{
		int record_size = 2 + _raw_search_buf[i][0] + _raw_search_buf[i][1];
		memcpy(&search_buf[offset], _raw_search_buf[i], record_size);
		offset += record_size;
	}

	send_packet(conn_fd, status, search_buf, search_buf_size);

	free(search_buf);

	return 1;
}
