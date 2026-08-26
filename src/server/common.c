
#include "common.h"

#include <stdbool.h>

bool check_username(const char *username, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (username[i] < 0x20 || username[i] > 0x7E)
		{
			return false;
		}
	}

	return true;
}
