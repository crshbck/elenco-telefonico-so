#include "login.h"

#include "../../common/utils.h"
#include "../../config.h"
#include "../../protocol.h"
#include "../crypto.h"
#include "../requests.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prompt_login(int conn_fd)
{
	printf(" Accesso\n");

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

		if (strlen(username) < MIN_USERNAME_LEN)
		{
			printf(" |  L'username deve avere una lunghezza minima di %d caratteri!\n",
				   MIN_USERNAME_LEN);
			continue;
		}

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

		status_t status = login(conn_fd, username, password_hash, strlen(username));

		switch (status)
		{
		case OK:
			printf(" Accesso eseguito con successo!\n\n");
			success = true;
			break;
		case SERVER_ERROR:
			printf(" Errore del server, riprova più tardi!\n");
			exit(0);
			break;
		case INVALID_CREDENTIALS:
			printf(" Credenziali errate!\n");
			break;
		default:
			printf(" (0x%02X) Errore sconosciuto, riprova più tardi!\n", status);
			break;
		}
	}
}
