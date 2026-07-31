
#include "query.h"

#include <stdlib.h>

#include "../../protocol.h"

user_t *query_user(const char *username, const char *password)
{
	user_t *user = malloc(sizeof(user_t));

	user->username = username;
	user->password = password;
	user->auth_level = USER;

	return user;
}
