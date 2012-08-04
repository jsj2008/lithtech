
// This is for simple but very convenient doubly linked lists..
// You need a DLink for the list head, and this just defines
// really common routines to insert, remove, and tie off.

#ifndef __DLINK_H__
#define __DLINK_H__


	class DLink;


	// These are only used internally in the engine (with lots of casting)
	// where memory is scarce for the head of a DLink list.
	typedef struct CheapDLink_t
	{
		DLink *m_pPrev, *m_pNext;
	} CheapDLink;



	#include "basetypes_de.h"


	// This goes thru and C++ deletes everything in a list.
	// You need a DLink *pCur, *pNext;
	#define CPP_DELETELIST(_list_pointer, type) \
	{\
		pCur = (_list_pointer)->m_Head.m_pNext;\
		while(pCur != &(_list_pointer)->m_Head)\
		{\
			pNext = pCur->m_pNext;\
			delete ((type*)pCur->m_pData);\
			pCur = pNext;\
		}\
		dl_InitList(_list_pointer);\
	}


	// Use these to declare blank DLinks and DLists.
	#define DECLARE_DLINK(name) \
		DLink name = {&(name), &(name), 0};

	#define DECLARE_DLIST(name) \
		DList name = {0, &name.m_Head, &name.m_Head, 0};


	class DLink
	{
	public:
		// Makes it easy to traverse the list in either direction.
		DLink* operator[](unsigned long i)
		{
			return ((DLink**)this)[i];
		}

		DLink *m_pPrev, *m_pNext;
		void *m_pData;
	};


	INLINE_FN void dl_TieOff(DLink *pLink)
	{
		pLink->m_pNext = pLink->m_pPrev = pLink;
	}

	INLINE_FN void dl_Insert(DLink *pAfter, DLink *pLink)
	{
		pLink->m_pPrev = pAfter;
		pLink->m_pNext = pAfter->m_pNext;
		pLink->m_pPrev->m_pNext = pLink->m_pNext->m_pPrev = pLink;
	}

	INLINE_FN void dl_Remove(DLink *pLink)
	{
		pLink->m_pPrev->m_pNext = pLink->m_pNext;
		pLink->m_pNext->m_pPrev = pLink->m_pPrev;
	}


	// You can use this list structure to wrap up a linked list
	// if you want to keep a count of the elements.
	typedef struct DList_t
	{
		unsigned long	m_nElements;
		DLink			m_Head;
	} DList;

	INLINE_FN void dl_InitList(DList *pList)
	{
		dl_TieOff(&pList->m_Head);
		pList->m_nElements = 0;
	}

	INLINE_FN void dl_AddAfter(DList *pList, DLink *pAfter, DLink *pLink, void *pObj)
	{
		pLink->m_pData = pObj;
		dl_Insert(pAfter, pLink);
		++pList->m_nElements;
	}

	INLINE_FN void dl_AddBefore(DList *pList, DLink *pBefore, DLink *pLink, void *pObj)
	{
		dl_AddAfter(pList, pBefore->m_pPrev, pLink, pObj);
	}

	INLINE_FN void dl_AddHead(DList *pList, DLink *pLink, void *pObj)
	{
		dl_AddAfter(pList, &pList->m_Head, pLink, pObj);
	}

	INLINE_FN void dl_AddTail(DList *pList, DLink *pLink, void *pObj)
	{
		dl_AddAfter(pList, pList->m_Head.m_pPrev, pLink, pObj);
	}

	INLINE_FN void dl_RemoveAt(DList *pList, DLink *pLink)
	{
		dl_Remove(pLink);
		--pList->m_nElements;
	}


#endif  
