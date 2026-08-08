#ifndef CONTACT_DB_H
#define CONTACT_DB_H

#include <stdbool.h>

bool init_contact_db();
bool add_contact(const char *name, const char *phone_number);
bool delete_contact(const char *name);

#endif
