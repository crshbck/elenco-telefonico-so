#ifndef REQUESTS_H
#define REQUESTS_H

typedef enum _status
{
	WRONG_CREDENTIALS,
	SERVER_ERROR,
	OK
} status;

status login(const char *username, const char *password);

#endif
