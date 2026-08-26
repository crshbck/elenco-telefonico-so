#ifndef CONTACT_DB_H
#define CONTACT_DB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool init_contact_db();
bool close_contact_db();

/// @retval 1 Contact added
/// @retval -1 I/O error
/// @retval -2 Corrupted database
int add_contact(const char *name, const char *phone_number, uint8_t name_length,
				uint8_t phone_number_length);

/// @retval 0 Contact not found
/// @retval 1 Contact deleted
/// @retval -1 I/O error
/// @retval -2 Corrupted database
int delete_contact(const char *name, uint8_t name_length);

/// @param[in]  query           Query string
/// @param[in]  query_length    Query length
/// @param[in]  limit           Matching contacts limit
/// @param[out] buf             Destination buffer for found contacts
/// @param[out] match_count     Pointer to matched contacts count
/// @retval  1               Success
/// @retval -1               I/O error
/// @retval -2               Corrupted database
int search_contact(const char *query, size_t query_length, uint8_t limit, char ***buf,
				   size_t *match_count);

void free_query_buffer(char **buf, size_t len);

#endif
