#include "../common/utils.h"
#include "../config.h"
#include "credentials.h"
#include "requests.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void prompt_login()
{
	bool success = false;

	while (!success)
	{
		printf(" Username: ");

		char username[MAX_USERNAME_LEN + 1];
		if (fgets(username, MAX_USERNAME_LEN + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		printf(" Password: ");

		char password[MAX_PASSWORD_LEN + 1];
		if (fgets(password, MAX_PASSWORD_LEN + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		printf("\n");

		if (login(username, password) != OK)
		{
			printf(" Username e password errati! Ritenta.\n");
		}
		else
		{
			printf(" Accesso eseguito con successo!\n");
			success = true;
		}
	}
}

void prompt_registration() {}

void prompt_auth()
{
	bool isopvalid = false;

	while (!isopvalid)
	{
		printf(" Seleziona l'operazione:\n\n"
			   " 1. Accedi\n"
			   " 2. Registrati\n"
			   " q. Esci\n\n"
			   " > ");

		char op = fgetc(stdin);
		fgetc(stdin);

		printf("\n");

		switch (op)
		{
		case '1':
			prompt_login();
			isopvalid = true;
			break;
		case '2':
			prompt_registration();
			isopvalid = true;
			break;
		case 'q':
			exit(0);
		default:
			printf(" Operazione non valida!\n");
			break;
		}
	}
}

int main(int argc, char **argv)
{
	printf("===== Elenco Telefonico =====\n\n");

	char username[MAX_USERNAME_LEN + 1];
	char password[MAX_PASSWORD_LEN + 1];

	if (access(CREDENTIALS_FILENAME, F_OK | R_OK | W_OK) == -1)
	{
		if (errno == EACCES)
		{
			error("Credentials file access error!");
		}

		prompt_auth();
	}
	else
	{
		if (load_credentials(username, password) == -1)
		{
			fprintf(stderr, "Credentials file is not valid!\n");
			exit(-1);
		}
		else
		{
			printf(" Trying to log in as '%s'\n", username);
		}
	}

	return 0;
}
