#include "search.h"

#include "../../common/utils.h"
#include "../../config.h"
#include "../../protocol.h"
#include "../requests.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prompt_search(int conn_fd)
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

	contact_t *buffer = malloc(sizeof(contact_t) * CONTACT_BUFFER_SIZE);

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
		free(buffer);
		return;
	}

	printf("(%zu) Contatti trovati:\n|\n", count);

	for (size_t i = 0; i < count; i++)
	{
		printf("| %s: %s\n", buffer[i].name, buffer[i].phone_number);
	}

	free(buffer);

	if (status == FEWER_RETURNED)
	{
		printf(" Mostrati i primi %zu risultati\n", count);
	}

	printf("============================="
		   "\n\n");
}
