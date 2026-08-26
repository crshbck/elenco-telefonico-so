#ifndef REQUESTS_H
#define REQUESTS_H

#include "../protocol.h"

auth_level_t getAuthLevel();
status_t login(const char *username, const char *password);
status_t signup(const char *username, const char *password);
status_t search_contact(contact *buffer, const int max, size_t *count, const char *name);
status_t add_contact(char *name, char *phone_number);

#endif
