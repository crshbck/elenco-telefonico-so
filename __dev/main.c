#include "user_db.h"

#include <assert.h>
#include <stdio.h>

int main()
{
	assert(init_user_db());

	user_t user = {
		.username = "Pippo", .password = "X123000000000000000000000000321X", .auth_level = ADMIN};

	assert(add_user(&user));

	auth_level_t auth_level;

	printf("%d %d\n", check_credentials("Pippo", "X123000000000000000000000000321X", &auth_level),
		   auth_level);
	printf("%d %d\n", check_credentials("Pippo", "X123000000000000000A00000000321X", &auth_level),
		   auth_level);

	return 0;
}
