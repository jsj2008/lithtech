#include "stringhelper.h"

// ----------------------------------------------------------
// SHAME ON ME.  I just ripped this STL stuff right outta MFC
// ----------------------------------------------------------

CPlex* CPlex::Create(CPlex*& pHead, uint32 nMax, uint32 cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p;
	LT_MEM_TRACK_ALLOC(p = (CPlex*) new int8[sizeof(CPlex) + nMax * cbElement],LT_MEM_TYPE_MISC);
			// may throw exception
	p->pNext = pHead;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
}

void CPlex::FreeDataChain()     // free this one and links
{
	CPlex* p = this;
	while (p != NULL)
	{
		int8* bytes = (int8*) p;
		CPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}

/////////////////////////////////////////////////////////////////////////////

CMapWordToPtr::CMapWordToPtr(int16 nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_pHashTable = LTNULL;
	m_nHashTableSize = 17;  // default size
	m_nCount = 0;
	m_pFreeList = LTNULL;
	m_pBlocks = LTNULL;
	m_nBlockSize = nBlockSize;
}

inline uint16 CMapWordToPtr::HashKey(uint32 key) const
{
	// default identity hash - works for most primitive values
	return ((uint16)(uint32)(int64)key) >> 4;
}

void CMapWordToPtr::InitHashTable(
	uint16 nHashSize, LTBOOL bAllocNow)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small)
{
	ASSERT(m_nCount == 0);
	ASSERT(nHashSize > 0);

	if (m_pHashTable != LTNULL)
	{
		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = LTNULL;
	}

	if (bAllocNow)
	{
		LT_MEM_TRACK_ALLOC(m_pHashTable = new CAssoc* [nHashSize],LT_MEM_TYPE_MISC);
		memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

void CMapWordToPtr::RemoveAll()
{
	if (m_pHashTable != LTNULL)
	{
		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = LTNULL;
	}

	m_nCount = 0;
	m_pFreeList = LTNULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = LTNULL;
}

CMapWordToPtr::~CMapWordToPtr()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Assoc helpers
// same as CList implementation except we store CAssoc's not CNode's
//    and CAssoc's are singly linked all the time

CMapWordToPtr::CAssoc*
CMapWordToPtr::NewAssoc()
{
	if (m_pFreeList == LTNULL)
	{
		// add another block
		CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMapWordToPtr::CAssoc));
		// chain them into free list
		CMapWordToPtr::CAssoc* pAssoc = (CMapWordToPtr::CAssoc*) newBlock->data();
		// free in reverse order to make it easier to debug
		pAssoc += m_nBlockSize - 1;
		for (int16 i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
		{
			pAssoc->pNext = m_pFreeList;
			m_pFreeList = pAssoc;
		}
	}
	ASSERT(m_pFreeList != LTNULL);  // we must have something

	CMapWordToPtr::CAssoc* pAssoc = m_pFreeList;
	m_pFreeList = m_pFreeList->pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);  // make sure we don't overflow


	pAssoc->key = 0;

	pAssoc->value = 0;

	return pAssoc;
}

void CMapWordToPtr::FreeAssoc(CMapWordToPtr::CAssoc* pAssoc)
{

	pAssoc->pNext = m_pFreeList;
	m_pFreeList = pAssoc;
	m_nCount--;
	ASSERT(m_nCount >= 0);  // make sure we don't underflow

	// if no more elements, cleanup completely
	if (m_nCount == 0)
		RemoveAll();
}

CMapWordToPtr::CAssoc*
CMapWordToPtr::GetAssocAt(int32 key, uint16& nHash) const
// find association (or return NULL)
{
	nHash = HashKey(key) % m_nHashTableSize;

	if (m_pHashTable == LTNULL)
		return LTNULL;

	// see if it exists
	CAssoc* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != LTNULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
			return pAssoc;
	}
	return LTNULL;
}



/////////////////////////////////////////////////////////////////////////////

LTBOOL CMapWordToPtr::Lookup(int32 key, void*& rValue) const
{
	uint16 nHash;
	CAssoc* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == LTNULL)
		return LTFALSE;  // not in map

	rValue = pAssoc->value;
	return LTTRUE;
}

void*& CMapWordToPtr::operator[](int32 key)
{
	uint16 nHash;
	CAssoc* pAssoc;
	if ((pAssoc = GetAssocAt(key, nHash)) == LTNULL)
	{
		if (m_pHashTable == LTNULL)
			InitHashTable(m_nHashTableSize);

		// it doesn't exist, add a new Association
		pAssoc = NewAssoc();

		pAssoc->key = key;
		// 'pAssoc->value' is a constructed object, nothing more

		// put into hash table
		pAssoc->pNext = m_pHashTable[nHash];
		m_pHashTable[nHash] = pAssoc;
	}
	return pAssoc->value;  // return new reference
}


LTBOOL CMapWordToPtr::RemoveKey(int32 key)
// remove key - return TRUE if removed
{
	if (m_pHashTable == LTNULL)
		return LTFALSE;  // nothing in the table

	CAssoc** ppAssocPrev;
	ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

	CAssoc* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != LTNULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
		{
			// remove it
			*ppAssocPrev = pAssoc->pNext;  // remove from list
			FreeAssoc(pAssoc);
			return LTTRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return LTFALSE;  // not found
}


/////////////////////////////////////////////////////////////////////////////
// Iterating

void CMapWordToPtr::GetNextAssoc(POSITION& rNextPosition,
	int32& rKey, void*& rValue) const
{
	ASSERT(m_pHashTable != LTNULL);  // never call on empty map

	CAssoc* pAssocRet = (CAssoc*)rNextPosition;
	ASSERT(pAssocRet != LTNULL);

	if (pAssocRet == (CAssoc*) BEFORE_START_POSITION)
	{
		// find the first association
		for (uint16 nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocRet = m_pHashTable[nBucket]) != LTNULL)
				break;
		ASSERT(pAssocRet != LTNULL);  // must find something
	}

	// find next association
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == LTNULL)
	{
		// go to next bucket

		for (uint16 nBucket = (HashKey(pAssocRet->key) % m_nHashTableSize) + 1;

		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != LTNULL)
				break;
	}

	rNextPosition = (POSITION) pAssocNext;

	// fill in return data
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}
