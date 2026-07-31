#ifndef DATABASE_H
#define DATABASE_H

#include "../src/protocol.h"

#include <stdbool.h>

bool init_db();
bool add_user(const user_t *user);
int check_credentials(const char *username, const char *password, auth_level_t *auth_level);

#endif
