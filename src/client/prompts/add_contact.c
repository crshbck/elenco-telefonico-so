#include "add_contact.h"

#include "../../common/utils.h"
#include "../../config.h"
#include "../../protocol.h"
#include "../requests.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prompt_add_contact(int conn_fd)
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
		break;
	default:
		printf(" Errore sconosciuto, riprova più tardi!\n");
		break;
	}
}
