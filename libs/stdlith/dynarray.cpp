
#include "dynarray.h"


uint32 MoArray_FindElementMemcmp(
	const void *pToFind,
	const void *pArray, 
	uint32 nElements, 
	uint32 elementSize)
{
	const char *pCur = (const char *)pToFind;
	uint32 i;


	for(i=0; i < nElements; i++)
	{
		if(memcmp(pCur, pToFind, elementSize) == 0)
			return i;
	
		pCur += elementSize;
	}

	return BAD_INDEX;
}
