#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "config.h"
#include <stddef.h>
#include <stdint.h>

#define STARTING_SEQ 0xA5

typedef struct __attribute__((__packed__)) _packet_header_t
{
	char opcode;
	uint16_t payload_size;
} packet_header_t;

typedef enum _status
{
	OK = 0x80,
	SERVER_ERROR = 0x81,
	MALFORMED_REQUEST = 0x82,
	NOT_FOUND = 0x83,
	UNAUTHORIZED = 0x84,
	INVALID_CREDENTIALS = 0x85,
	USERNAME_TAKEN = 0x86,
	ALL_RETURNED = 0x87,
	FEWER_RETURNED = 0x88,
	CONTACT_ALREADY_EXISTS = 0x89,
} status_t;

typedef enum _auth_level
{
	ANONYMOUS = 0x00,
	USER = 0x01,
	ADMIN = 0x02
} auth_level_t;

typedef struct _user_t
{
	char *username;
	char *password;
	auth_level_t auth_level;
} user_t;

typedef struct _contact
{
	char name[MAX_CONTACT_NAME_LEN];
	char phone_number[MAX_PHONE_NUMBER_LENGTH];
} contact;

typedef enum _packet_type
{
	LOGIN = 0x00,
	REGISTER = 0x01,
	INSERT = 0x02,
	DELETE = 0x03,
	SEARCH = 0x04
} packet_type;

#endif
