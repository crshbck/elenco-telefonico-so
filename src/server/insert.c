#include "insert.h"

#include "../common/utils.h"
#include "database/contact_db.h"
#include "sender.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int handle_insert(const packet_header_t *header, int conn_fd)
{
	char buffer[header->payload_size];

	ssize_t read_bytes = recv_exact(conn_fd, buffer, header->payload_size, 0);

	if (read_bytes == -1)
	{
		perror("Error during receive!");
		return -1;
	}

	// at least 1B name_length + name 1B + phone_number 1B
	if (read_bytes < header->payload_size || read_bytes < 3)
	{
		if (!send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		return 0;
	}

	unsigned int name_length = buffer[0];
	unsigned int phone_number_length = header->payload_size - 1 - buffer[0];

	if (name_length == 0 || name_length > MAX_CONTACT_NAME_LEN || phone_number_length == 0 ||
		phone_number_length > MAX_PHONE_NUMBER_LENGTH)
	{
		if (!send_packet(conn_fd, MALFORMED_REQUEST, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		return 0;
	}

	switch (add_contact(&buffer[1], &buffer[1 + name_length], name_length, phone_number_length))
	{
	case 1:
		printf("Added contact '");

		for (int i = 0; i < name_length; i++)
		{
			printf("%c", buffer[1 + i]);
		}

		printf("'\n");

		if (!send_packet(conn_fd, OK, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
			return -1;
		}
		return 1;
		break;
	case -1:
		if (!send_packet(conn_fd, SERVER_ERROR, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
		}
		perror("Database I/O error! Terminating...");
		exit(-1);
		break;
	case -2:
		if (!send_packet(conn_fd, SERVER_ERROR, NULL, 0))
		{
			fprintf(stderr, "Error sending packet!\n");
		}
		fprintf(stderr, "Database is corrupted! Terminating...\n");
		exit(-1);
		break;
	}

	return -1;
}
