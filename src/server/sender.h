#ifndef SENDER_H
#define SENDER_H

#include "../protocol.h"
#include <stdbool.h>
#include <unistd.h>

bool send_packet(int conn_fd, status_t status, char *payload, uint16_t payload_size);
ssize_t discard_bytes(int fd, size_t count);

#endif
