//------------------------------------------------------------------
//
//	FILE	: BDEFS.CPP
//
//	PURPOSE	: Base definition code body
//
//	CREATED	: 3rd November 1995
//
//------------------------------------------------------------------

// Includes....

#include "bdefs.h"


int32	g_DebugLevel=0;
char *g_ReturnErrString = "LT ERROR: %s returned %s (%s)";


#ifdef BDEFS_SIM
	void dsi_PrintToConsole(char *pStr, ...)
	{
	}

	void dsi_OnReturnError(int level)
	{
	}
#endif


#ifdef DALLOC_SIM
	void* dalloc(DWORD size)
	{
		return malloc(size);
	}

	void* dalloc_z(DWORD size)
	{
		void *ret;

		if(ret = malloc(size))
		{
			memset(ret, 0, size);
		}

		return ret;
	}

	void dfree(void *ptr)
	{
		free(ptr);
	}
#endif

	  