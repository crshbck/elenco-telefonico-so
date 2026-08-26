#include "../common/utils.h"
#include "../config.h"
#include "credentials.h"
#include "requests.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int conn_fd;

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
		case OK:
			printf(" Accesso eseguito con successo!\n\n");
			success = true;
			break;
		case SERVER_ERROR:
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
		case OK:
			printf(" Registrazione eseguita con successo!\n\n");
			success = true;
			break;
		case INVALID_CREDENTIALS:
			printf(" Username già registrato! Ritenta.\n");
			break;
		case SERVER_ERROR:
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

	status_t status = search_contact(buffer, CONTACT_BUFFER_SIZE, &count, contact_name);

	switch (status)
	{
	case SERVER_ERROR:
		printf(" Errore del server, riprova più tardi!\n");
		exit(0);
		break;
	case UNAUTHORIZED:
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

	if (status == FEWER_RETURNED)
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
		case CONTACT_ALREADY_EXISTS:
			printf(" Il contatto già esiste!\n");
			break;
		case SERVER_ERROR:
			printf(" Errore del server, riprova più tardi!\n");
			exit(0);
			break;
		case OK:
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

void prompt_operation(auth_level_t level)
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

int connect_to_server(const char *ip)
{
	struct sockaddr_in server_addr = {0};

	conn_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (conn_fd < 0)
	{
		perror("Errore creazione socket");
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);

	if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
	{
		perror("Indirizzo IP non valido");
		close(conn_fd);
		return -1;
	}

	// 3. Connessione al server (avvia il 3-way handshake TCP)
	if (connect(conn_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		perror("Connessione fallita");
		close(conn_fd);
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	printf("===== Elenco Telefonico =====\n\n");

	char username[MAX_USERNAME_LEN + 1];
	char password[MAX_PASSWORD_LEN + 1];

	char *ip = argv[0];

	if (connect_to_server(ip) == -1)
	{
		return -1;
	}

	printf("Connesso a %s:%d\n", ip, SERVER_PORT);

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

	auth_level_t level = getAuthLevel();

	if (level == ANONYMOUS)
	{
		printf(" Non hai nessun permesso!\n");
		exit(0);
	}

	prompt_operation(level);

	return 0;
}
