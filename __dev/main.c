#include "contact_db.h"
#include "user_db.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main()
{
	// assert(init_user_db());

	// user_t user = {
	// 	.username = "Pippo", .password = "X123000000000000000000000000321X", .auth_level = ADMIN};

	// // assert(add_user(&user));

	// auth_level_t auth_level;

	// printf("%d", check_credentials("Pippo", "X123000000000000000000000000321X", &auth_level));
	// printf(" %d\n", auth_level);

	// printf("%d", check_credentials("Pippo", "A123000000000000000000000000321X", &auth_level));
	// printf(" %d\n", auth_level);

	// assert(close_user_db());

	assert(init_contact_db());

	char *name = "Mario Rossi";
	char *phone_number = "3012333321";
	// assert(add_contact(name, phone_number, strlen(name), strlen(phone_number)));

	char *name1 = "Giuseppe Bianchi";
	char *phone_number1 = "3112333321";
	// assert(add_contact(name1, phone_number1, strlen(name1), strlen(phone_number1)));

	char *name2 = "Francesco Quaglia";
	char *phone_number2 = "3212333321";
	// assert(add_contact(name2, phone_number2, strlen(name2), strlen(phone_number2)));

	char *name3 = "Francesco Totti";
	char *phone_number3 = "3101010101";
	// assert(add_contact(name3, phone_number3, strlen(name3), strlen(phone_number3)));

	char *query = "francesco";

	char **buf;
	size_t match_count;

	printf("status: %d\n", search_contact(query, strlen(query), 100, &buf, &match_count));

	printf("Found %llu \n", match_count);

	for (int i = 0; i < match_count; i++)
	{
		int name_len = buf[i][0];
		int pn_len = buf[i][1];

		for (int j = 0; j < name_len; j++)
		{
			printf("%c", buf[i][2 + j]);
		}

		printf("\t");

		for (int j = 0; j < pn_len; j++)
		{
			printf("%c", buf[i][name_len + 2 + j]);
		}

		printf("\n");
	}

	// assert(delete_contact(name, strlen(name)) == 1);

	assert(close_contact_db());

	return 0;
}
