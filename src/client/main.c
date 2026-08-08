#include "../common/utils.h"
#include "../config.h"
#include "credentials.h"
#include "requests.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void prompt_login()
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

		printf(" |  Password: ");

		char password[MAX_PASSWORD_LEN + 1];
		if (fgets(password, MAX_PASSWORD_LEN + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		printf("\n");

		switch (login(username, password))
		{
		case LOGIN_OK:
			printf(" Accesso eseguito con successo!\n\n");
			success = true;
			break;
		case LOGIN_SERVER_ERROR:
			printf(" Errore del server, riprova più tardi!\n");
			break;
		default:
			printf(" Errore sconosciuto, riprova più tardi!\n");
			break;
		}
	}
}

void prompt_registration()
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

		printf(" |  Password: ");

		char password[MAX_PASSWORD_LEN + 1];
		if (fgets(password, MAX_PASSWORD_LEN + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		printf("\n");

		switch (signup(username, password))
		{
		case SIGNUP_OK:
			printf(" Registrazione eseguita con successo!\n\n");
			success = true;
			break;
		case SIGNUP_USERNAME_TAKEN:
			printf(" Username già registrato! Ritenta.\n");
			break;
		case SIGNUP_SERVER_ERROR:
			printf(" Errore del server, riprova più tardi!\n");
			break;
		default:
			printf(" Errore sconosciuto, riprova più tardi!\n");
			break;
		}
	}
}

void prompt_auth()
{
#ifdef DEBUG
	login("admin", "admin");
	return;
#endif

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
			printf(" Operazione non valida!\n\n");
			break;
		}
	}
}

void prompt_search()
{
	printf("Inserisci il nome del contatto:\n"
		   "\n"
		   " > ");

	char contact_name[MAX_CONTACT_NAME_LEN + 1];
	if (fgets(contact_name, MAX_CONTACT_NAME_LEN + 1, stdin) == NULL)
	{
		error("Input error!");
	}

	const size_t CONTACT_BUFFER_SIZE = 12;

	contact buffer[CONTACT_BUFFER_SIZE];

	size_t count;

	search_status status = search_contact(buffer, CONTACT_BUFFER_SIZE, &count, contact_name);

	switch (status)
	{
	case SEARCH_SERVER_ERROR:
		printf(" Errore del server, riprova più tardi!\n");
		exit(0);
		break;
	case SEARCH_UNAUTHORIZED:
		printf(" Non sei autorizzato ad eseguire questa operazione!\n");
		exit(0);
		break;
	default:
		break;
	}
	printf("\n");

	if (count == 0)
	{
		printf(" Nessun contatto trovato!\n\n");
		return;
	}

	printf("(%zu) Contatti trovati:\n|\n", count);

	for (size_t i = 0; i < count; i++)
	{
		printf("| %s: %s\n", buffer[i].name, buffer[i].phone_number);
	}

	if (status == SEARCH_FEWER_RETURNED)
	{
		printf(" Mostrati i primi %zu risultati\n", count);
	}

	printf("============================="
		   "\n\n");
}

void prompt_add_contact()
{
	bool success = false;

	while (!success)
	{
		printf(" | Nome: ");

		char name[MAX_CONTACT_NAME_LEN + 1];
		if (fgets(name, MAX_CONTACT_NAME_LEN + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		printf(" | Numero di telefono: ");

		char phone_number[MAX_PHONE_NUMBER_LENGTH + 1];
		if (fgets(phone_number, MAX_PHONE_NUMBER_LENGTH + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		printf("\n");

		switch (add_contact(name, phone_number))
		{
		case ADD_CONTACT_ALREAD_EXISTS:
			printf(" Il contatto già esiste!\n");
			break;
		case ADD_CONTACT_SERVER_ERROR:
			printf(" Errore del server, riprova più tardi!\n");
			exit(0);
			break;
		case ADD_CONTACT_OK:
			printf(" Contatto aggiunto con successo!\n"
				   "============================="
				   "\n\n");
			success = true;
			break;
		default:
			printf(" Errore sconosciuto, riprova più tardi!\n");
			break;
		}
	}
}

void prompt_operation(auth_level level)
{
	while (1)
	{
		printf(" Seleziona l'operazione:\n\n"
			   " 1. Ricerca un contatto\n");

		if (level == ADMIN)
		{
			printf(" 2. Aggiungi un contatto\n");
		}

		printf(" q. Esci\n\n"
			   " > ");

		char op = fgetc(stdin);
		fgetc(stdin);

		printf("\n");

		switch (op)
		{
		case '1':
			prompt_search();
			break;
		case '2':
			if (level != ADMIN)
			{
				printf(" Operazione non valida!\n\n");
				break;
			}
			prompt_add_contact();
			break;
		case 'q':
			exit(0);
		default:
			printf(" Operazione non valida!\n\n");
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

			if (login(username, password) == -1)
			{
				printf(" Credentials inside credentials file are not valid!\n");
			}
		}
	}

	prompt_auth();

	auth_level level = getAuthLevel();

	if (level == ANONYMOUS)
	{
		printf(" Non hai nessun permesso!\n");
		exit(0);
	}

	prompt_operation(level);

	return 0;
}
