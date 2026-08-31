#include "requests.h"

#include "../common/utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

auth_level_t auth_level;

auth_level_t get_auth_level() { return auth_level; }

status_t login(int conn_fd, const char *username, const uint8_t *password, size_t username_length)
{
	// starting seq (1B) + opcode (1B) + payload_len (2B) + username (?B) + password (32B)
	char buffer[1 + 1 + 2 + username_length + 32];

	buffer[0] = STARTING_SEQ;
	buffer[1] = 0x00;
	buffer[2] = (32 + username_length) >> 8;
	buffer[3] = (32 + username_length) & 0x00FF;
	memcpy(&buffer[4], username, username_length);
	memcpy(&buffer[4 + username_length], password, 32);

	if (send_exact(conn_fd, buffer, sizeof(buffer), 0) < sizeof(buffer))
	{
		return SERVER_ERROR;
	}

	unsigned char header[3];

	ssize_t res = recv_exact(conn_fd, header, 3, 0);

	if (res == -1)
	{
		printf("Il server non risponde, riprova più tardi!\n");
		exit(0);
	}

	if (res < 3)
	{
		return SERVER_ERROR;
	}

	if (header[0] == OK)
	{
		res = recv_exact(conn_fd, header, 1, 0);

		if (res == -1)
		{
			printf("Il server non risponde, riprova più tardi!\n");
			exit(0);
		}

		if (res < 1)
		{
			return SERVER_ERROR;
		}

		auth_level = header[0];
		return OK;
	}

	return header[0];
}

status_t register_user(int conn_fd, const char *username, const uint8_t *password,
					   size_t username_length)
{
	// starting seq (1B) + opcode (1B) + payload_len (2B) + username (?B) + password (32B)
	char buffer[1 + 1 + 2 + username_length + 32];

	buffer[0] = STARTING_SEQ;
	buffer[1] = 0x01;
	buffer[2] = (32 + username_length) >> 8;
	buffer[3] = (32 + username_length) & 0x00FF;
	memcpy(&buffer[4], username, username_length);
	memcpy(&buffer[4 + username_length], password, 32);

	if (send_exact(conn_fd, buffer, sizeof(buffer), 0) < sizeof(buffer))
	{
		return SERVER_ERROR;
	}

	unsigned char header[3];

	ssize_t res = recv_exact(conn_fd, header, 3, 0);

	if (res == -1)
	{
		printf("Il server non risponde, riprova più tardi!\n");
		exit(0);
	}

	if (res < 3)
	{
		return SERVER_ERROR;
	}

	return header[0];
}

status_t search_contact(int conn_fd, contact_t *output_buffer, const uint8_t max,
						size_t *match_count, const char *query, size_t query_length)
{
	// starting seq (1B) + opcode (1B) + payload_len (2B) + limit (1B) + query (?B)
	char buffer[1 + 1 + 2 + 1 + query_length];

	buffer[0] = STARTING_SEQ;
	buffer[1] = 0x04;
	buffer[2] = (1 + query_length) >> 8;
	buffer[3] = (1 + query_length) & 0x00FF;
	buffer[4] = max;
	memcpy(&buffer[5], query, query_length);

	if (send_exact(conn_fd, buffer, sizeof(buffer), 0) < sizeof(buffer))
	{
		return SERVER_ERROR;
	}

	unsigned char header[3];

	ssize_t res = recv_exact(conn_fd, header, 3, 0);

	if (res == -1)
	{
		printf("Il server non risponde, riprova più tardi!\n");
		exit(0);
	}

	if (res < 3)
	{
		return SERVER_ERROR;
	}

	size_t size = ((header[1] << 8) + header[2]);

	if (size == 0)
	{
		*match_count = 0;
		return header[0];
	}

	uint8_t *_contact_buffer = malloc(sizeof(uint8_t) * size);

	if (_contact_buffer == NULL)
	{
		perror("Malloc error");
		exit(-1);
	}

	res = recv_exact(conn_fd, _contact_buffer, size, 0);

	if (res == -1)
	{
		printf("Il server non risponde, riprova più tardi!\n");
		exit(0);
	}

	if (res < size)
	{
		free(_contact_buffer);
		return SERVER_ERROR;
	}

	size_t offset = 0;
	*match_count = 0;

	while (offset < size && *match_count < max)
	{
		// sanity check
		if (offset + 2 > size)
		{
			break;
		}

		uint8_t name_length = _contact_buffer[offset];
		uint8_t phone_number_length = _contact_buffer[offset + 1];

		// sanity check
		if (offset + 2 + name_length + phone_number_length > size)
		{
			// declared sizes are not true
			return SERVER_ERROR;
		}

		if (name_length > MAX_CONTACT_NAME_LEN || phone_number_length > MAX_PHONE_NUMBER_LENGTH)
		{
			return SERVER_ERROR;
		}

		// copy inside buffer
		memcpy(output_buffer[*match_count].name, &_contact_buffer[offset + 2], name_length);
		output_buffer[*match_count].name[name_length] = '\0';

		memcpy(output_buffer[*match_count].phone_number, &_contact_buffer[offset + 2 + name_length],
			   phone_number_length);

		output_buffer[*match_count].phone_number[phone_number_length] = '\0';

		// check if all characters are printable
		if (!is_printable(output_buffer[*match_count].name, name_length) ||
			!is_printable(output_buffer[*match_count].phone_number, phone_number_length))
		{
			return SERVER_ERROR;
		}

		// step record
		offset += 2 + name_length + phone_number_length;
		(*match_count)++;
	}

	free(_contact_buffer);

	return header[0];
}

status_t add_contact(int conn_fd, const char *name, const char *phone_number, size_t name_length,
					 size_t phone_number_length)
{
	// starting seq (1B) + opcode (1B) + payload_len (2B) + name_length (1B) + name (?B) +
	// phone_number (?B)
	size_t payload_length = 1 + name_length + phone_number_length;

	char buffer[1 + 1 + 2 + payload_length];

	buffer[0] = STARTING_SEQ;
	buffer[1] = 0x02;
	buffer[2] = payload_length >> 8;
	buffer[3] = payload_length & 0x00FF;
	buffer[4] = name_length;

	memcpy(&buffer[5], name, name_length);
	memcpy(&buffer[5 + name_length], phone_number, phone_number_length);

	if (send_exact(conn_fd, buffer, sizeof(buffer), 0) < sizeof(buffer))
	{
		return SERVER_ERROR;
	}

	unsigned char header[3];

	ssize_t res = recv_exact(conn_fd, header, 3, 0);

	if (res == -1)
	{
		printf("Il server non risponde, riprova più tardi!\n");
		exit(0);
	}

	if (res < 3)
	{
		return SERVER_ERROR;
	}

	return header[0];
}

status_t delete_contact(int conn_fd, const char *query, size_t query_length)
{
	// starting seq (1B) + opcode (1B) + payload_len (2B) + query (?B)
	char buffer[1 + 1 + 2 + query_length];

	buffer[0] = STARTING_SEQ;
	buffer[1] = 0x03;
	buffer[2] = query_length >> 8;
	buffer[3] = query_length & 0x00FF;

	memcpy(&buffer[4], query, query_length);

	if (send_exact(conn_fd, buffer, sizeof(buffer), 0) < sizeof(buffer))
	{
		return SERVER_ERROR;
	}

	unsigned char header[3];

	ssize_t res = recv_exact(conn_fd, header, 3, 0);

	if (res == -1)
	{
		printf("Il server non risponde, riprova più tardi!\n");
		exit(0);
	}

	if (res < 3)
	{
		return SERVER_ERROR;
	}

	return header[0];
}
