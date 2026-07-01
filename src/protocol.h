#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "config.h"
#include <stddef.h>

typedef enum _login_status
{
	LOGIN_WRONG_CREDENTIALS,
	LOGIN_SERVER_ERROR,
	LOGIN_OK
} login_status;

typedef enum _signup_status
{
	SIGNUP_USERNAME_TAKEN,
	SIGNUP_SERVER_ERROR,
	SIGNUP_OK
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

#endif
