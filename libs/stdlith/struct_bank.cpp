
#include "stdlith.h"
#include "struct_bank.h"


int32 g_bDebugStructBanks = 0;



void sb_Init(StructBank *pBank, uint32 structSize, uint32 cacheSize)
{
	pBank->m_StructSize = structSize;
	
	pBank->m_AlignedStructSize = structSize;
	if(pBank->m_AlignedStructSize < sizeof(StructLink))
		pBank->m_AlignedStructSize = sizeof(StructLink);

	if(pBank->m_AlignedStructSize % 4 != 0)
	{
		pBank->m_AlignedStructSize += (4 - (pBank->m_AlignedStructSize % 4));
 	}
	
	pBank->m_nPages = 0;
	pBank->m_nTotalObjects = 0;
	pBank->m_CacheSize = cacheSize;
	pBank->m_PageHead = NULL;
	pBank->m_FreeListHead = NULL;
}


int32 sb_Init2(StructBank *pBank, uint32 structSize, uint32 cacheSize, uint32 nPreallocations)
{
	sb_Init(pBank, structSize, cacheSize);

	#ifdef _DEBUG
		// No preallocations if debugging..
		if(g_bDebugStructBanks)
			return 1;
	#endif

	return sb_AllocateNewStructPage(pBank, nPreallocations);
}


void sb_FreeAll(StructBank *pBank)
{
	StructBankPage *pPage;
	StructLink *pStruct;
	uint8 *pDataPos;
	uint32 count;

	pBank->m_FreeListHead = NULL;

	pPage = pBank->m_PageHead;
	while(pPage)
	{
		#ifdef _DEBUG
			memset(pPage->m_Data, 0xEA, pBank->m_AlignedStructSize * pPage->m_nObjects);
		#endif

		pDataPos = (uint8*)pPage->m_Data;
		count = pPage->m_nObjects;
		while(count--)
		{
			pStruct = (StructLink*)pDataPos;
			pStruct->m_pSLNext = pBank->m_FreeListHead;
			pBank->m_FreeListHead = pStruct;
			pDataPos += pBank->m_AlignedStructSize;
		}
		
		pPage = pPage->m_pNext;
	}
}


void sb_Term2(StructBank *pBank, int32 bCheckObjects)
{
	StructBankPage *pPage;
	StructBankPage *pNext;

	#ifdef _DEBUG
		uint32 freeListCount;
		StructLink *pCur;

		if(bCheckObjects)
		{
			// Make sure everything is freed.
			freeListCount = 0;
			for(pCur=pBank->m_FreeListHead; pCur; pCur=pCur->m_pSLNext)
			{
				++freeListCount;
			}

			ASSERT(freeListCount == pBank->m_nTotalObjects);
		}
	#endif

	pPage = pBank->m_PageHead;
	while(pPage)
	{
		pNext = pPage->m_pNext;
		g_DefAlloc.Free(pPage);
		pPage = pNext;
	}

	pBank->m_PageHead = NULL;
	pBank->m_FreeListHead = NULL;
	pBank->m_nPages = 0;
	pBank->m_nTotalObjects = 0;
}


void sb_Term(StructBank *pBank)
{
	sb_Term2(pBank, 0);
}


int32 sb_AllocateNewStructPage(StructBank *pBank, uint32 nAllocations)
{
	StructLink *pStruct;
	StructBankPage *pPage;
	uint8 *pDataPos;
	uint32 count;

	
	if(nAllocations == 0)
		return 1;

	// Allocate a new page.
	pPage = (StructBankPage*)g_DefAlloc.Alloc((pBank->m_AlignedStructSize*nAllocations) + (sizeof(StructBankPage)-sizeof(uint32)));
	if(!pPage)
		return 0;

	pPage->m_pNext = pBank->m_PageHead;
	pPage->m_nObjects = nAllocations;

	pBank->m_PageHead = pPage;

	++pBank->m_nPages;
	pBank->m_nTotalObjects += nAllocations;

	// Put its contents into the free list.
	pDataPos = (uint8*)pPage->m_Data;
	count = nAllocations;
	while(count--)
	{
		pStruct = (StructLink*)pDataPos;
		pStruct->m_pSLNext = pBank->m_FreeListHead;
		pBank->m_FreeListHead = pStruct;
		pDataPos += pBank->m_AlignedStructSize;
	}

	return 1;
}


LTBOOL sb_IsObjectAllocated(StructBank *pBank, void *pObj)
{
	uint8 *pBytes;
	uint32 count;

	// The first 4 bytes are our StructLink, but the rest should be 0xEA.

	pBytes = (uint8*)pObj;
	pBytes += sizeof(StructLink*);

	if(pBank->m_StructSize > sizeof(StructLink*))
	{
		count = pBank->m_StructSize - sizeof(StructLink*);
		while(count--)
		{
			if(*pBytes != 0xEA)
				return TRUE;
		
			++pBytes;
		}
	
		// All bytes are 0xEA.. this object is freed.
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

