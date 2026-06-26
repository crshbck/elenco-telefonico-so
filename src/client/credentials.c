#include "credentials.h"
#include "../config.h"

#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CREDENTIALS_SEPARATOR 0x1F // Ascii repr for RECORD_SEPARATOR

int load_credentials(char *usern, char *passw)
{
	int cred_fd = open(CREDENTIALS_FILENAME, O_RDONLY, 0600);

	if (cred_fd == -1)
	{
		return -1;
	}

	char buffer[MAX_USERNAME_LEN + 1 + MAX_PASSWORD_LEN + 1];

	int read_bytes = read(cred_fd, buffer, MAX_USERNAME_LEN + 1 + MAX_PASSWORD_LEN);

	buffer[read_bytes] = '\0';

	char delim[2] = {CREDENTIALS_SEPARATOR, '\0'};

	buffer[MAX_USERNAME_LEN + 1 + MAX_PASSWORD_LEN] = '\0';

	char *_usern = strtok(buffer, delim);
	char *_passw = strtok(NULL, delim);

	if (_usern == NULL || _passw == NULL)
	{
		return -1;
	}

	if (strlen(_usern) > MAX_USERNAME_LEN || strlen(_passw) > MAX_PASSWORD_LEN)
	{
		return -1;
	}

	strcpy(usern, _usern);
	strcpy(passw, _passw);

	return 0;
}
