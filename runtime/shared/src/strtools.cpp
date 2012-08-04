//////////////////////////////////////////////////////////////////////////////
// String tool implementation

#include "bdefs.h"
#include "strtools.h"

uint32 st_GetHashCode(const char *pString)
{
	uint32 nResult = 0;
	for(; *pString; ++pString)
	{
		nResult *= 31;
		nResult += *pString;
	}

	return nResult;
}

uint32 st_GetHashCode_ic(const char *pString)
{
	uint32 nResult = 0;
	for(; *pString; ++pString)
	{
		nResult *= 29;
		nResult += toupper(*pString) - 'A';
	}

	return nResult;
}
