#ifndef SEARCH_H
#define SEARCH_H

#include "../protocol.h"

int handle_search(const packet_header_t *header, int conn_fd);

#endif
