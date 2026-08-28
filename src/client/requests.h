#ifndef REQUESTS_H
#define REQUESTS_H

#include "../protocol.h"
#include <stdint.h>

auth_level_t get_auth_level();
status_t login(int conn_fd, const char *username, const uint8_t *password, size_t username_length);
status_t register_user(int conn_fd, const char *username, const uint8_t *password,
					   size_t username_length);
status_t search_contact(int conn_fd, contact_t *output_buffer, const uint8_t max,
						size_t *match_count, const char *query, size_t query_length);
status_t add_contact(int conn_fd, const char *name, const char *phone_number, size_t name_length,
					 size_t phone_number_length);
status_t delete_contact(int conn_fd, const char *query, size_t query_length);

#endif
