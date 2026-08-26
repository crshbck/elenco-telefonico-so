#include "requests.h"

auth_level_t level;

auth_level_t getAuthLevel() { return level; }

status_t login(const char *username, const char *password)
{
	// todo

	level = ADMIN;

	return OK;
}

status_t signup(const char *username, const char *password)
{
	// todo

	return OK;
}

status_t search_contact(contact *buffer, const int max, size_t *count, const char *name)
{
	buffer[0] = (contact) {
		.name = "Mario Rossi",
		.phone_number = "+39 312 345 6789",
	};

	buffer[1] = (contact) {
		.name = "Luca Bianchi",
		.phone_number = "+39 333 987 6543",
	};

	buffer[2] = (contact) {
		.name = "Elena Verdi",
		.phone_number = "+39 347 112 2334",
	};

	buffer[3] = (contact) {
		.name = "Francesco Neri",
		.phone_number = "+39 320 555 7788",
	};

	*count = 4;
	return SEARCH_OK;
}

status_t add_contact(char *name, char *phone_number) { return ADD_CONTACT_OK; }
