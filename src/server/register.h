#ifndef REGISTER_H
#define REGISTER_H

#include "../protocol.h"

#include <stdbool.h>

int handle_register(const packet_header_t *header, user_t *user, int conn_fd);

#endif
