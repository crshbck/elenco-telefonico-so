#include "registration.h"

#include "../../common/utils.h"
#include "../../config.h"
#include "../../protocol.h"
#include "../crypto.h"
#include "../requests.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prompt_registration(int conn_fd)
{
	printf(" Registrazione\n");

	bool success = false;

	while (!success)
	{
		printf(" |  Username: ");

		char username[MAX_USERNAME_LEN + 1];
		if (fgets(username, MAX_USERNAME_LEN + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		// remove terminator
		username[strcspn(username, "\n")] = '\0';

		printf(" |  Password: ");

		char password[MAX_PASSWORD_LEN + 1];
		if (fgets(password, MAX_PASSWORD_LEN + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		// remove terminator
		password[strcspn(password, "\n")] = '\0';

		uint8_t password_hash[32];

		compute_sha256(password, strlen(password), password_hash);

		printf("\n");

		switch (register_user(conn_fd, username, password_hash, strlen(username)))
		{
		case OK:
			printf(" Registrazione eseguita con successo!\n\n");
			exit(0);
			break;
		case USERNAME_TAKEN:
			printf(" Username già registrato! Ritenta.\n");
			break;
		case SERVER_ERROR:
			printf(" Errore del server, riprova più tardi!\n");
			exit(0);
			break;
		default:
			printf(" Errore sconosciuto, riprova più tardi!\n");
			break;
		}
	}
}
