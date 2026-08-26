
#include "delete.h"

#include "../common/utils.h"
#include "database/contact_db.h"
#include "sender.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int handle_delete(const packet_header_t *header, int conn_fd)
{
	char name[header->payload_size];

	ssize_t read_bytes = recv_exact(conn_fd, name, header->payload_size, 0);

	if (read_bytes == -1)
	{
		perror("Error during receive!");
		return -1;
	}

	if (read_bytes < header->payload_size)
	{
		if (!send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		return 0;
	}

	printf("Deleting contact ");

	for (int i = 0; i < header->payload_size; i++)
	{
		printf("%c", name[i]);
	}

	printf("\n");

	switch (delete_contact(name, header->payload_size))
	{
	case 0:
		send_packet(conn_fd, NOT_FOUND, NULL, 0);
		return 0;
		break;
	case 1:
		send_packet(conn_fd, OK, NULL, 0);
		return 1;
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

	return -1;
}
