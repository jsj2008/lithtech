
#include "stdlith.h"
#include "struct_bank.h"
#include "linklist.h"


StructBank g_LinkListBank;
int g_LinkListRefCount=0;


LPOS LinkList_FindElementMemcmp(
	const void *pHead,
	const void *pToFind,
	uint32 elementSize)
{
	CGenLLNode *pGenCur, *pGenHead;

	if(pHead)
	{
		pGenHead = pGenCur = (CGenLLNode*)pHead;
		do
		{
			if(memcmp(pGenCur->m_Data, pToFind, elementSize) == 0)
				return pGenCur;

			pGenCur = pGenCur->m_pNext;
		} while(pGenCur != pGenHead);
	}

	return NULL;
}
