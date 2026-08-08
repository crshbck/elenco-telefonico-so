
#include "../src/config.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int contact_db_fd;

void init_contact_db() { contact_db_fd = open(CONTACT_DB_FILENAME, O_RDWR | O_CREAT, 0640); }
