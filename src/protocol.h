#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "config.h"
#include <cstdint>
#include <stddef.h>
#include <stdint.h>

typedef struct __attribute__((__packed__)) _packet_header_t
{
	char opcode;
	uint16_t payload_size;
} packet_header_t;

typedef enum _login_status
{
	LOGIN_OK,
	LOGIN_WRONG_CREDENTIALS,
	LOGIN_SERVER_ERROR
} login_status;

typedef enum _signup_status
{
	SIGNUP_OK,
	SIGNUP_USERNAME_TAKEN,
	SIGNUP_SERVER_ERROR
} signup_status;

typedef enum _auth_level
{
	ANONYMOUS,
	USER,
	ADMIN
} auth_level;

typedef struct _contact
{
	char name[MAX_CONTACT_NAME_LEN];
	char phone_number[MAX_PHONE_NUMBER_LENGTH];
} contact;

typedef enum _search_status
{
	SEARCH_OK,
	SEARCH_ALL_RETURNED,
	SEARCH_FEWER_RETURNED,
	SEARCH_UNAUTHORIZED,
	SEARCH_SERVER_ERROR
} search_status;

typedef enum _add_contact_status
{
	ADD_CONTACT_OK,
	ADD_CONTACT_ALREAD_EXISTS,
	ADD_CONTACT_UNAUTHORIZED,
	ADD_CONTACT_SERVER_ERROR
} add_contact_status;

typedef enum __attribute__((packed)) _packet_type
{
	LOGIN,
	REGISTER,
	ADD_CONTACT,
	SEARCH
} packet_type;

#endif
