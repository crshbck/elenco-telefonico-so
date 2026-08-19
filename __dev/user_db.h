#ifndef USER_DB_H
#define USER_DB_H

#include "../src/protocol.h"

#include <stdbool.h>

bool init_user_db();
bool close_user_db();
int add_user(const user_t *user);
int check_credentials(const char *username, const char *password, auth_level_t *auth_level);

#endif
