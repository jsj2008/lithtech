//////////////////////////////////////////////////////////////////////////////
// Banked List implementation

#include "BankedList.h"

#include <stdio.h>

// A page of items
// (Internal class for internal use only..)
template <typename T>
class CBankedListBank
{
public:

	CBankedListBank();

private:
    typedef CBankedList<T> t_Parent;
    typedef CBankedListBank<T> t_This;
	typedef T t_Type;

#ifdef __PSX2
    friend class t_Parent;
    friend class t_Parent::t_Iter;
#else
    friend t_Parent;
    friend t_Parent::t_Iter;
#endif

	// Is this bank currently full?
	LTBOOL IsFull();
	// Is this bank currently empty?
	LTBOOL IsEmpty();
	// Is this bank currently packed? (i.e. are the used items in contiguous usage?)
	LTBOOL IsPacked();
	// Is this index in use?
	LTBOOL IsUsed(uint32 nIndex);
	// Get the index of pPtr (returns k_SizeOfBank if it isn't in this bank)
	uint32 GetIndexAtPtr(t_Type *pPtr);
	// Get object nIndex
	t_Type &GetRefAtIndex(uint32 nIndex);
	// How many items are currently in use?
	uint32 GetSize();
	// What's your minimum in-use index?  (returns k_SizeOfBank if empty)
	uint32 GetFirstUsed();
	// What's your maximum in-use index?  (returns k_SizeOfBank if empty)
	uint32 GetLastUsed();
	// What's your minimum available index?  (returns k_SizeOfBank if full)
	uint32 GetFirstOpen();
	// What's your maximum in-use index?  (returns k_SizeOfBank if full)
	uint32 GetLastOpen();
	// Remove the item at index nIndex
	void Remove(uint32 nIndex);
	// Allocate an item
    T *New(typename t_Parent::t_NewOp eOp);
	// Read-only bank pointer access
	t_This *GetNext();
	t_This *GetPrev();
	// Remove this bank from the list
	void RemoveList();
	// Insert this bank into the list after pTail
	void AppendList(t_This *pTail);
	// Insert this bank into the list before pHead
	void InsertList(t_This *pHead);
private: // Intended for internal access only
	enum { k_DataPitch = ((sizeof(t_Type) + (sizeof(int) - 1)) / sizeof(int)) };
	enum { k_SizeOfData = t_Parent::k_SizeOfBank * k_DataPitch };
	// The usage flags for this bank
	uint32 m_nUsage;
	// Pointer to the next/prev banks in the list
	t_This *m_pNextBank, *m_pPrevBank;
	// The actual data (Note : This is an int array to avoid the ctor/dtor calls of the
	// actual type.  Also, this conveniently provides word alignment to the allocations..)
	int m_aData[k_SizeOfData];
};

//////////////////////////////////////////////////////////////////////////////
// CBankedList<T>::t_Iter implementation

template <typename T>
CBankedListIter<T>::CBankedListIter() : m_pCurBank(LTNULL), m_nIndex(0)
{
}

template <typename T>
CBankedListIter<T>::CBankedListIter(const t_This &iOther) : m_pCurBank(iOther.m_pCurBank), m_nIndex(iOther.m_nIndex)
{
}

template <typename T>
CBankedListIter<T>::CBankedListIter(t_Bank *pBank, uint32 nIndex) : m_pCurBank(pBank), m_nIndex(nIndex)
{
}

template <typename T>
LTBOOL CBankedListIter<T>::IsValid()
{
	// Invalid bank?  Not valid..
	if (!m_pCurBank)
		return LTFALSE;
	// Invalid index?  Not valid..
	if (m_nIndex >= t_Parent::k_SizeOfBank)
		return LTFALSE;
	// You're valid, as long as the bank says you're in use
	return m_pCurBank->IsUsed(m_nIndex);
}

template <typename T>
T &CBankedListIter<T>::DeRef()
{
	// Not pointing at anything?   Uh...  Make something up...
	if (!m_pCurBank)
	{
		return *((T*)LTNULL);
	}

	return m_pCurBank->GetRefAtIndex(m_nIndex);
}

template <typename T>
void CBankedListIter<T>::Next()
{
	if (!m_pCurBank)
		return;

	do
	{
		if (m_nIndex < t_Parent::k_SizeOfBank)
			++m_nIndex;
		else
		{
			m_pCurBank = m_pCurBank->GetNext();
			m_nIndex = 0;
		}
	} while (m_pCurBank && (!m_pCurBank->IsUsed(m_nIndex)));
}

template <typename T>
void CBankedListIter<T>::Prev()
{
	if (!m_pCurBank)
		return;

	do
	{
		if (m_nIndex > 0)
			--m_nIndex;
		else
		{
			m_pCurBank = m_pCurBank->GetPrev();
			if (m_pCurBank)
				m_nIndex = m_pCurBank->GetLastUsed();
		}
	} while (m_pCurBank && (!m_pCurBank->IsUsed(m_nIndex)));
}

template <typename T>
void CBankedListIter<T>::Copy(const t_This &iOther)
{
	m_pCurBank = iOther.m_pCurBank;
	m_nIndex = iOther.m_nIndex;
}

template <typename T>
CBankedListBank<T> *CBankedListIter<T>::GetBank()
{
	return m_pCurBank;
}

template <typename T>
uint32 CBankedListIter<T>::GetIndex()
{
	return m_nIndex;
}

//////////////////////////////////////////////////////////////////////////////
// CBankedList<T>::t_Bank implementation

template <typename T>
CBankedListBank<T>::CBankedListBank() : m_nUsage(0), m_pNextBank(LTNULL), m_pPrevBank(LTNULL)
{
}

template <typename T>
LTBOOL CBankedListBank<T>::IsFull()
{
	return m_nUsage == 0xFFFFFFFF;
}

template <typename T>
LTBOOL CBankedListBank<T>::IsEmpty()
{
	return m_nUsage == 0;
}

template <typename T>
LTBOOL CBankedListBank<T>::IsPacked()
{
	// Returns TRUE if the "used" bits are contiguous
	return (m_nUsage & (m_nUsage + 1)) == 0;
}

template <typename T>
LTBOOL CBankedListBank<T>::IsUsed(uint32 nIndex)
{
	return (m_nUsage & (1 << nIndex)) != 0;
}

template <typename T>
uint32 CBankedListBank<T>::GetIndexAtPtr(T *pPtr)
{
	if (((int*)pPtr >= m_aData) && ((int*)pPtr < &m_aData[k_SizeOfData]))
		return ((int*)pPtr - m_aData) / k_DataPitch;
	else
		return t_Parent::k_SizeOfBank;
}

template <typename T>
T &CBankedListBank<T>::GetRefAtIndex(uint32 nIndex)
{
	// Note : It is guaranteed that this won't be called with nIndex > k_SizeOfBank
	// due to the iterator being the only one to call this function
	return *((T*)(&m_aData[nIndex * k_DataPitch]));
}

// How many items are currently in use?
template <typename T>
uint32 CBankedListBank<T>::GetSize()
{
	// This is a keen little trick I found in a Dr. Dobbs for counting bits without any branches..
	uint32 nTemp;
	nTemp = ((m_nUsage & 0xAAAAAAAA) >> 1) + (m_nUsage & 0x55555555);
	nTemp = ((nTemp & 0xCCCCCCCC) >> 2) + (nTemp & 0x33333333);
	nTemp = ((nTemp & 0xF0F0F0F0) >> 4) + (nTemp & 0x0F0F0F0F);
	nTemp = ((nTemp & 0xFF00FF00) >> 8) + (nTemp & 0x00FF00FF);
	return ((nTemp & 0xFFFF0000) >> 16) + (nTemp & 0x0000FFFF);
}

template <typename T>
uint32 CBankedListBank<T>::GetFirstUsed()
{
	// Note : I'm sure there's a more clever way of doing this...
	uint32 nIndex = 0;
	uint32 nMask = 1;
	while (nIndex < t_Parent::k_SizeOfBank)
	{
		if ((m_nUsage & nMask) != 0)
			return nIndex;
		++nIndex;
		nMask = nMask << 1;
	}
	// If it got here, it's empty
	return nIndex;
}

template <typename T>
uint32 CBankedListBank<T>::GetLastUsed()
{
	// Note : I'm sure there's a more clever way of doing this...
	uint32 nIndex = t_Parent::k_SizeOfBank - 1;
	uint32 nMask = 1 << nIndex;
	while (nIndex > 0)
	{
		if ((m_nUsage & nMask) != 0)
			return nIndex;
		--nIndex;
		nMask = nMask >> 1;
	}
	// If it got here, it's either 0 or empty
	return (m_nUsage == 1) ? 0 : t_Parent::k_SizeOfBank;
}

template <typename T>
uint32 CBankedListBank<T>::GetFirstOpen()
{
	// Note : I'm sure there's a more clever way of doing this...
	uint32 nIndex = 0;
	uint32 nMask = 1;
	while (nIndex < t_Parent::k_SizeOfBank)
	{
		if ((m_nUsage & nMask) == 0)
			return nIndex;
		++nIndex;
		nMask = nMask << 1;
	}
	// If it got here, it's full
	return nIndex;
}

template <typename T>
uint32 CBankedListBank<T>::GetLastOpen()
{
	// Note : I'm sure there's a more clever way of doing this...
	uint32 nIndex = t_Parent::k_SizeOfBank - 1;
	uint32 nMask = 1 << nIndex;
	while (nIndex > 0)
	{
		if ((m_nUsage & nMask) == 0)
			return nIndex;
		--nIndex;
		nMask = nMask >> 1;
	}
	// If it got here, it's either 0 or empty
	return (m_nUsage == 1) ? 0 : t_Parent::k_SizeOfBank;
}

template <typename T>
void CBankedListBank<T>::Remove(uint32 nIndex)
{
	m_nUsage &= ~(1 << nIndex);
}

template <typename T>
T* CBankedListBank<T>::New(typename t_Parent::t_NewOp eOp)
{
	// Get our index
	uint32 nIndex = t_Parent::k_SizeOfBank;
	if (eOp == t_Parent::eNewOp_Pack)
	{
		// Fail if we're full
		if (IsFull())
			return LTNULL;
		nIndex = GetFirstOpen();
	}
	else if (eOp == t_Parent::eNewOp_Append)
	{
		// Fail if we can't append..
		if (IsUsed(t_Parent::k_SizeOfBank - 1) || (GetNext() != LTNULL))
			return LTNULL;

		if (IsEmpty())
			nIndex = 0;
		else
			nIndex = GetLastUsed() + 1;
	}

	// Mark it as used
	m_nUsage |= (1 << nIndex);

	// Return the pointer..
	return &GetRefAtIndex(nIndex);
}

template <typename T>
CBankedListBank<T> *CBankedListBank<T>::GetNext()
{
	return m_pNextBank;
}

template <typename T>
CBankedListBank<T> *CBankedListBank<T>::GetPrev()
{
	return m_pPrevBank;
}

template <typename T>
void CBankedListBank<T>::RemoveList()
{
	if (m_pPrevBank)
		m_pPrevBank->m_pNextBank = m_pNextBank;
	if (m_pNextBank)
		m_pNextBank->m_pPrevBank = m_pPrevBank;
}

template <typename T>
void CBankedListBank<T>::AppendList(t_This *pTail)
{
	if (pTail)
	{
		if (pTail->m_pNextBank)
			pTail->m_pNextBank->m_pPrevBank = this;
		m_pNextBank = pTail->m_pNextBank;
		m_pPrevBank = pTail;
		pTail->m_pNextBank = this;
	}
	else
	{
		m_pNextBank = LTNULL;
		m_pPrevBank = LTNULL;
	}
}

template <typename T>
void CBankedListBank<T>::InsertList(t_This *pHead)
{
	if (pHead)
	{
		if (pHead->m_pPrevBank)
			pHead->m_pPrevBank->m_pNextBank = this;
		m_pNextBank = pHead;
		m_pPrevBank = pHead->m_pPrevBank;
		pHead->m_pPrevBank = this;
	}
	else
	{
		m_pNextBank = LTNULL;
		m_pPrevBank = LTNULL;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CBankedList<T> implementation

template <typename T>
CBankedList<T>::CBankedList() : m_pBankHead(LTNULL), m_pBankTail(LTNULL), m_pEmptyHead(LTNULL), m_nSize(0)
{
}

template <typename T>
CBankedList<T>::~CBankedList()
{
	Term();
}

template <typename T>
void CBankedList<T>::Init()
{
	Term();
}

template <typename T>
void CBankedList<T>::Term()
{
	if (m_pEmptyHead)
	{
		DeleteBankList(m_pEmptyHead);
		m_pEmptyHead = LTNULL;
	}
	if (m_pBankHead)
	{
		DeleteBankList(m_pBankHead);
		m_pBankHead = LTNULL;
	}
	m_pBankTail = LTNULL;
	m_nSize = 0;
}

template <typename T>
typename CBankedList<T>::t_Iter CBankedList<T>::GetHead()
{
	return t_Iter(m_pBankHead, (m_pBankHead) ? m_pBankHead->GetFirstUsed() : 0);
}

template <typename T>
typename CBankedList<T>::t_Iter CBankedList<T>::GetTail()
{
	return t_Iter(m_pBankTail, (m_pBankTail) ? m_pBankTail->GetLastUsed() : 0);
}

template <typename T>
typename CBankedList<T>::t_Iter CBankedList<T>::Remove(const t_Iter &iPos)
{
	// Make sure we've got a valid position
	if (!iPos.IsValid())
		return iPos;

	// Set up the return value
	t_Iter iRet(iPos);

	// Get the item we're pointing at
	t_Bank *pBank = iRet.GetBank();
	uint32 nIndex = iRet.GetIndex();
	// Move to the next item
	++iRet;
	// Remove the item from the bank
	pBank->Remove(nIndex);
	// Remember that we removed one
	--m_nSize;

	// Remove the bank from the list if it's empty
	if (pBank->IsEmpty())
		MoveToEmptyList(pBank);

	return iRet;
}

template <typename T>
typename CBankedList<T>::t_Iter CBankedList<T>::Find(T *pPtr)
{
	// Start at the first bank
	t_Bank *pFinger = m_pBankHead;
	uint32 nIndex = 0;
	while (pFinger)
	{
		// Try to find it in this bank
		nIndex = pFinger->GetIndexAtPtr(pPtr);
		// Jump out of the loop if we found it
		if (nIndex < k_SizeOfBank)
			break;
		// Move to the next bank
		pFinger = pFinger->GetNext();
	}
	// Return the result
	return t_Iter(pFinger, nIndex);
}

template <typename T>
void CBankedList<T>::Remove(T *pPtr)
{
	Remove(Find(pPtr));
}

template <typename T>
void CBankedList<T>::Append(const T &_Item)
{
	new(this, eNewOp_Append) T(_Item);
}

template <typename T>
void CBankedList<T>::Delete(t_Type *pPtr, bool bCallDtor)
{
	if (!pPtr)
		return;

#ifdef __PSX2
	// Call the dtor
	if (bCallDtor)
        delete pPtr;
//        pPtr->~T();
    else
        free(pPtr);
#else

	// Find the pointer in the list
	t_Iter iPtr = Find(pPtr);

	// Jump out if we didn't find it
	if (!iPtr.IsValid())
	{
		ASSERT(!"CBankedList<T>::Delete was called on an unknown pointer");
		return;
	}

	// Call the dtor
	if (bCallDtor)
		pPtr->~T();

	// Delete it
	iPtr.GetBank()->Remove(iPtr.GetIndex());

	// Remember that we removed one
	--m_nSize;

	// Remove the bank if it's empty
	if (iPtr.GetBank()->IsEmpty())
		MoveToEmptyList(iPtr.GetBank());

	// Report
	/*
	if (g_pLTClient)
	{
		g_pLTClient->CPrint("BankedList::Delete(%d) : %d", sizeof(t_Type), GetSize());
	}
	*/
#endif
}

template <typename T>
void *operator new(size_t, CBankedList<T> &, T* ptr) { return ptr; } // Internal ctor helper
template <typename T>
void operator delete(void*, CBankedList<T> &cBank, T* ptr) { cBank.Delete(ptr); }

template <typename T>
T *CBankedList<T>::New(t_NewOp eOp, bool bCallCtor)
{
#ifdef __PSX2
    T *pNewItem;
    pNewItem = new T;
#else
	// Get our bank
	t_Bank *pBank = LTNULL;
	if (eOp == eNewOp_Pack)
	{
		// Find the first available bank
		pBank = m_pBankHead;
		while (pBank && pBank->IsFull())
			pBank = pBank->GetNext();
	}
	else if (eOp == eNewOp_Append)
	{
		pBank = m_pBankTail;
		// Get a new bank if this one's been used all the way..
		if (pBank && pBank->IsUsed(k_SizeOfBank - 1))
			pBank = LTNULL;
	}

	// Get a new bank if we need to..
	if (!pBank)
	{
		pBank = GetNewBank();
	}

	// Fail if we just can't do it...
	if (!pBank)
	{
		ASSERT(!"Unable to allocate a bank in CBankedList<T>::New");
		return LTNULL;
	}

	// Get a new one from the bank
	T *pNewItem = pBank->New(eOp);

	// Remember that we added one...
	if (pNewItem)
		++m_nSize;

	// Call the ctor
	::new(*this, pNewItem) T;

	// Report
	/*
	if (g_pLTClient)
	{
		g_pLTClient->CPrint("BankedList::New(%d) : %d", sizeof(t_Type), GetSize());
	}
	*/
#endif
	return pNewItem;
}

template <typename T>
LTBOOL CBankedList<T>::IsEmpty()
{
	return m_pBankHead == LTNULL;
}

template <typename T>
uint32 CBankedList<T>::GetSize()
{
	return m_nSize;
}

template <typename T>
uint32 CBankedList<T>::GetFootprint()
{
	// Start out with the size of the list...
	uint32 nSize = sizeof(*this);

	// Add in the active banks
	t_Bank *pFinger = m_pBankHead;
	while (pFinger)
	{
		nSize += sizeof(*pFinger);
		pFinger = pFinger->GetNext();
	}

	// Add in the empty banks
	t_Bank *pFinger = m_pEmptyHead;
	while (pFinger)
	{
		nSize += sizeof(*pFinger);
		pFinger = pFinger->GetNext();
	}
}

template <typename T>
float CBankedList<T>::GetUsage()
{
	// Count the number of slots used/total
	uint32 nUsedSlots = 0;
	uint32 nTotalSlots = 0;

	// Only count the active banks
	t_Bank *pFinger = m_pBankHead;
	while (pFinger)
	{
		nTotalSlots += k_SizeOfBank;
		nUsedSlots += pFinger->GetSize();
		pFinger = pFinger->GetNext();
	}

	// Empty lists are considered entirely used
	if (!nTotalSlots)
		return 1.0f;
	else
		return (float)nUsedSlots / (float)nTotalSlots;
}

template <typename T>
void CBankedList<T>::DeleteBankList(t_Bank *pHead)
{
	t_Bank *pFinger = pHead;
	while (pFinger)
	{
		t_Bank *pDeleteMe = pFinger;
		pFinger = pFinger->GetNext();

		debug_delete(pDeleteMe);
	}
}

template <typename T>
void CBankedList<T>::MoveToEmptyList(t_Bank *pBank)
{
	// Update the head pointer
	if (pBank == m_pBankHead)
		m_pBankHead = m_pBankHead->GetNext();
	// Update the tail pointer
	if (pBank == m_pBankTail)
		m_pBankTail = m_pBankTail->GetPrev();
	// Remove the bank from the active list
	pBank->RemoveList();
	// Add it to the empty list
	pBank->InsertList(m_pEmptyHead);
	m_pEmptyHead = pBank;
}

template <typename T>
typename CBankedList<T>::t_Bank *CBankedList<T>::GetNewBank()
{
	t_Bank *pResult;
	// If we've got an empty list available...
	if (m_pEmptyHead)
	{
		// Use the head of the empty list
		pResult = m_pEmptyHead;
		// Move the empty list forward
		m_pEmptyHead = m_pEmptyHead->GetNext();
		// Remove it from the empty list
		pResult->RemoveList();
	}
	// Otherwise make a new one
	else
	{
		pResult = debug_new(t_Bank);
	}

	// Add it to the end of the list
	pResult->AppendList(m_pBankTail);
	// Point the tail at it
	m_pBankTail = pResult;
	// Point the head at it if we're empty
	if (!m_pBankHead)
		m_pBankHead = pResult;

	// Here ya go...
	return pResult;
}

template <typename T>
LTBOOL CBankedList<T>::PreAlloc(uint32 nMinItems)
{
	// Calculate how many banks we'd need..
	uint32 nBanksNeeded = (nMinItems + k_SizeOfBank - 1) / k_SizeOfBank;
	// Find out what we've currently got allocated
	t_Bank *pFinger = m_pEmptyHead;
	while (nBanksNeeded && pFinger)
	{
		--nBanksNeeded;
		pFinger = pFinger->GetNext();
	}
	// Allocate new banks
	while (nBanksNeeded)
	{
		t_Bank *pNewBank = debug_new(t_Bank);
		// Add them to the empty list
		pNewBank->InsertList(m_pEmptyHead);
		m_pEmptyHead = pNewBank;
	}
}

template <typename T>
void CBankedList<T>::Flush()
{
	if (m_pEmptyHead)
	{
		DeleteBankList(m_pEmptyHead);
		m_pEmptyHead = LTNULL;
	}
}