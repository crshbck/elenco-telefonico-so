#ifndef REQUESTS_H
#define REQUESTS_H

#include "../protocol.h"

auth_level getAuthLevel();
login_status login(const char *username, const char *password);
signup_status signup(const char *username, const char *password);
search_status search_contact(contact *buffer, const int max, size_t *count, const char *name);
add_contact_status add_contact(char *name, char *phone_number);

#endif
