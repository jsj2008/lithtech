#include "bdefs.h"
#include <malloc.h>
#include <string.h>
#include "ltbasetypes.h"

// This module just implements dalloc and dfree for the tools.

void* dalloc(uint32 size)
{
	if(size == 0)
		return 0;
	else
		return malloc(size);
}


void* dalloc_z(uint32 size)
{
	void *ptr;

	if(size == 0)
	{
		return 0;
	}
	else
	{
		ptr = malloc(size);
		if(ptr)
		{
			memset(ptr, 0, size);
		}

		return ptr;
	}
}


void dfree(void *ptr)
{
	free(ptr);
}


