//////////////////////////////////////////////////////////////////////////////
// Game implementation of the standard allocator

#include "stdafx.h"
#include <stdlib.h>

void* DefStdlithAlloc(uint32 size) 
{
	if(size == 0)
		return NULL;

	return malloc((size_t)size);
}

void DefStdlithFree(void *ptr)
{
	if(ptr)
	{
		free(ptr);
	}
}
