#include "delete_contact.h"

#include "../../common/utils.h"
#include "../../config.h"
#include "../../protocol.h"
#include "../requests.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prompt_delete_contact(int conn_fd)
{

	printf(" | Nome: ");

	char name[MAX_CONTACT_NAME_LEN + 1];
	if (fgets(name, MAX_CONTACT_NAME_LEN + 1, stdin) == NULL)
	{
		error("Input error!");
	}

	// remove terminator
	name[strcspn(name, "\n")] = '\0';

	printf("\n");

	switch (delete_contact(conn_fd, name, strlen(name)))
	{
	case NOT_FOUND:
		printf(" Contatto non trovato!\n");
		break;
	case SERVER_ERROR:
		printf(" Errore del server, riprova più tardi!\n");
		exit(0);
		break;
	case OK:
		printf(" Contatto rimosso con successo!\n"
			   "============================="
			   "\n\n");
		break;
	default:
		printf(" Errore sconosciuto, riprova più tardi!\n");
		break;
	}
}
