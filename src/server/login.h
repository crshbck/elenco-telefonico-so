#ifndef LOGIN_H
#define LOGIN_H

#include "../protocol.h"

#include <stdbool.h>

void handle_login(const packet_header_t *header, user_t *user, int conn_fd);

#endif
