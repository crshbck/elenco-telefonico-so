#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void error(const char *m)
{
	perror(m);
	exit(-1);
}
