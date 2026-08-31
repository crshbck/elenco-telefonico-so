#include "../config.h"

#include "./prompts/add_contact.h"
#include "./prompts/delete_contact.h"
#include "./prompts/login.h"
#include "./prompts/registration.h"
#include "./prompts/search.h"
#include "requests.h"

#include <arpa/inet.h>
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

#define fflush(stdin) int c;while ((c = getchar()) != '\n' && c != EOF);

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
			prompt_login(conn_fd);
			isopvalid = true;
			break;
		case '2':
			prompt_registration(conn_fd);
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

void prompt_operation(auth_level_t level)
{
	while (1)
	{
		printf(" Seleziona l'operazione:\n\n"
			   " 1. Ricerca un contatto\n");

		if (level == ADMIN)
		{
			printf(" 2. Aggiungi un contatto\n");
			printf(" 3. Elimina un contatto\n");
		}

		printf(" q. Esci\n\n"
			   " > ");

		char op = fgetc(stdin);
		fgetc(stdin);

		printf("\n");

		switch (op)
		{
		case '1':
			prompt_search(conn_fd);
			break;
		case '2':
			if (level != ADMIN)
			{
				printf(" Operazione non valida!\n\n");
				break;
			}
			prompt_add_contact(conn_fd);
			break;
		case '3':
			if (level != ADMIN)
			{
				printf(" Operazione non valida!\n\n");
				break;
			}
			prompt_delete_contact(conn_fd);
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
