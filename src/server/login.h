#ifndef LOGIN_H
#define LOGIN_H

#include "../protocol.h"

#include <stdbool.h>

int handle_login(const packet_header_t *header, user_t *user, int conn_fd);

int read_username_password(const packet_header_t *header, char **username, char **password,
						   int *username_length, int conn_fd);

#endif
