#include "../common/utils.h"
#include "../config.h"
#include "crypto.h"
#include "requests.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

		switch (login(conn_fd, username, password_hash, strlen(username)))
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

		switch (signup(conn_fd, username, password_hash, strlen(username)))
		{
		case OK:
			printf(" Registrazione eseguita con successo!\n\n");
			exit(0);
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
	// #ifdef DEBUG
	// 	login("admin", "admin");
	// 	return;
	// #endif

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
	printf("Inserisci il nome del contatto (vuoto per mostrarne il massimo):\n"
		   "\n"
		   " > ");

	char query[MAX_CONTACT_NAME_LEN + 1];
	if (fgets(query, MAX_CONTACT_NAME_LEN + 1, stdin) == NULL)
	{
		error("Input error!");
	}

	// remove terminator
	query[strcspn(query, "\n")] = '\0';

	const size_t CONTACT_BUFFER_SIZE = 12;

	contact_t buffer[CONTACT_BUFFER_SIZE];

	size_t count;

	status_t status =
		search_contact(conn_fd, buffer, CONTACT_BUFFER_SIZE, &count, query, strlen(query));

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

		// remove terminator
		name[strcspn(name, "\n")] = '\0';

		printf(" | Numero di telefono: ");

		char phone_number[MAX_PHONE_NUMBER_LENGTH + 1];
		if (fgets(phone_number, MAX_PHONE_NUMBER_LENGTH + 1, stdin) == NULL)
		{
			error("Input error!");
		}

		// remove terminator
		phone_number[strcspn(phone_number, "\n")] = '\0';

		printf("\n");

		switch (add_contact(conn_fd, name, phone_number, strlen(name), strlen(phone_number)))
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
		fprintf(stderr, "Indirizzo IP '%s' non valido\n", ip);
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

	if (argc < 2)
	{
		printf("Utilizzo: %s <server_ip>\n", argv[0]);
		exit(-1);
	}

	char *ip = argv[1];

	if (connect_to_server(ip) == -1)
	{
		return -1;
	}

	printf("Connesso a %s:%d\n", ip, SERVER_PORT);

	prompt_auth();

	auth_level_t level = get_auth_level();

	if (level == ANONYMOUS)
	{
		printf(" Non hai nessun permesso!\n");
		exit(0);
	}
	else if (level == USER)
	{
		printf("Livello di autorizzazione: UTENTE\n");
	}
	else if (level == ADMIN)
	{
		printf("Livello di autorizzazione: ADMIN\n");
	}
	else
	{
		printf("Errore\n");
		exit(-1);
	}

	prompt_operation(level);

	return 0;
}
