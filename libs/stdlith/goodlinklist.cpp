
#include "goodlinklist.h"


GPOS GLinkedList_FindElementMemcmp(
	CGLLNode *pHead,
	const void *pToFind,
	uint32 elementSize)
{
	CGLLNode *pCur;

	if(pHead)
	{
		pCur = pHead;
		do
		{
			if(memcmp(pCur, pToFind, elementSize) == 0)
				return pCur;

			pCur = pCur->m_pGNext;
		} while(pCur != pHead);
	}

	return NULL;
}


